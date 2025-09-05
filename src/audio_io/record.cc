#include "audio_io/record.h"
#include "portaudio.h"
#include "core/types.h"

int AJ::io::record::Recorder::recordCallback(const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags, void *userData){

    AudioData* data = (AudioData*) userData;
    
    const float *input_callback = (const float*)inputBuffer;    
    
    AJ::utils::Buffer* buffer = nullptr;
    
    while(!buffer){
        buffer = data->pBufferPool->pop(data->errHandler);
    }

    std::memcpy(buffer->data, input_callback, sizeof(float) * framesPerBuffer * buffer->channels);

    buffer->frames = framesPerBuffer;

    while(!data->pQueue->push(buffer)){
        const std::string message = "Error: pushing buffer failed in recordCallback, queue is full";
        data->errHandler.onError(AJ::error::Error::RecordingError, message);
    }

    if(data->pStopFlag->flag.load(std::memory_order_acquire)){
        return paComplete;
    }

    return paContinue;
}

void AJ::io::record::Recorder::diskWriter(){
    pStreamer->write(pAudioData->errHandler);
}

bool AJ::io::record::Recorder::initRecorder(){
    PaStreamParameters inputParameters;
    PaError err = paNoError;

    err = Pa_Initialize();
    if(err != paNoError){
        const std::string message = "Can't Initialize the error object for recording.";
        pAudioData->errHandler.onError(AJ::error::Error::ResourceAllocationFailed, message);
        return false;
    }

    inputParameters.device = Pa_GetDefaultInputDevice();

    if(inputParameters.device == paNoDevice){
        const std::string message = "Can't find the audio input device.";
        pAudioData->errHandler.onError(AJ::error::Error::ResourceAllocationFailed, message);
        return false;
    }

    inputParameters.channelCount = mAudioInfo.channels;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = nullptr;

    err = Pa_OpenStream(
        &mAudioInfo.stream, 
        &inputParameters,
        nullptr, // outputParameters
        mAudioInfo.samplerate,
        mAudioInfo.frames_per_buffer,
        paClipOff,
        recordCallback,
        pAudioData.get()
    );

    if(err != paNoError){
        const std::string message = "Can't open a stream.";
        pAudioData->errHandler.onError(AJ::error::Error::ResourceAllocationFailed, message);
        return false;
    }

    return true;
}

bool AJ::io::record::Recorder::record(AJ::utils::IEventHandler& evHandler){
    if(!initRecorder()){
        return false;
    }

    // wait until there is atleast 2 threads available.
    while(pThreadPool->available() < 2){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    //* start the disk writer thread.
    pThreadPool->enqueue([&](){
        diskWriter();
    });

    // wait for safety.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    //* start recording.
    PaError err = Pa_StartStream(mAudioInfo.stream);

    if(err != paNoError){
        const std::string message = "Can't start recording.";
        pAudioData->errHandler.onError(AJ::error::Error::RecordingError, message);
        return false;
    }
    
    evHandler.onProcess(pAudioData->errHandler, pThreadPool, pStopFlag);

    // After user stops, wait until PortAudio stream finishes
    while ((err = Pa_IsStreamActive(mAudioInfo.stream)) == 1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    err = Pa_CloseStream(mAudioInfo.stream);

    if(err != paNoError){
        const std::string message = "Can't close stream.";
        pAudioData->errHandler.onError(AJ::error::Error::RecordingError, message);
        return false;
    }

    return true;
}