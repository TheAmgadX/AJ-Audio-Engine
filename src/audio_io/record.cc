#include "audio_io/record.h"
#include "portaudio.h"
#include "core/types.h"

int AJ::io::record::Recorder::recordCallback(const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags, void *userData){

        
    //* 1- cast userData to data buffer.
    AudioData* data = (AudioData*) userData;

    if(!data->reading.load(std::memory_order_relaxed));{
        return paComplete;
    }

    const float *input_callback = (const float*)inputBuffer;

    float *output_callback = (float*)outputBuffer;

    // write frames will handle the buffer overflow.
    size_t written_frames = data->buffer->writeFrames(input_callback, framesPerBuffer);

    //! if the written frames is less than the frames per buffer log it.
    if(written_frames != framesPerBuffer){
        //TODO: use logger here.
    }

    return paContinue;
}

bool AJ::io::record::Recorder::record(AJ::error::IErrorHandler& errHandler,
    AJ::io::IEventHandler& evHandler){
    PaStreamParameters inputParameters;
    PaError err = paNoError;

    err = Pa_Initialize();
    if(err != paNoError){
        const std::string message = "Can't Initialize the error object for recording.";
        errHandler.onError(AJ::error::Error::ResourceAllocationFailed, message);
        return false;
    }

    inputParameters.device = Pa_GetDefaultInputDevice();

    if(inputParameters.device == paNoDevice){
        const std::string message = "Can't find the audio input device.";
        errHandler.onError(AJ::error::Error::ResourceAllocationFailed, message);
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
        &pAudioData
    );

    if(err != paNoError){
        const std::string message = "Can't open a stream.";
        errHandler.onError(AJ::error::Error::ResourceAllocationFailed, message);
        return false;
    }

    //* here we start the threads.
    err = Pa_StartStream(mAudioInfo.stream);

    if(err != paNoError){
        const std::string message = "Can't start recording.";
        errHandler.onError(AJ::error::Error::RecordingError, message);
        return false;
    }
    
    while((err = Pa_IsStreamActive(mAudioInfo.stream)) == 1){
        //TODO: evHandler.OnProcess(); this function responsible to stop the recording.
    }

    //? after finishing the current operations update it.
    pAudioData->reading.store(false, std::memory_order_seq_cst);        

    //* here we wait for threads.
    err = Pa_CloseStream(mAudioInfo.stream);
    if(err != paNoError){
        const std::string message = "Can't close stream.";
        errHandler.onError(AJ::error::Error::RecordingError, message);
        return false;
    }

    return true;
}