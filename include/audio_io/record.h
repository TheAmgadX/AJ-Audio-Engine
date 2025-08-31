#pragma once 
#include <memory>

#include "portaudio.h"
#include "core/types.h"
#include "core/error_handler.h"
#include "audio_io.h"
#include "core/ring_buffer.h"



namespace AJ::io::record {

class AudioMetaData {
public:
    int samplerate;
    int frames_per_buffer;
    int channels;
    bool is_recording;
    std::string recording_start_time;
    PaStream* stream;
    int mBufferSizePerChan;

    AudioMetaData(){}

    AudioMetaData(int rate): samplerate(rate) {
        mBufferSizePerChan = samplerate * BUFFER_SECONDS;    
    }
};

class AudioData {
public:
    std::shared_ptr<AJ::utils::RingBuffer> buffer;
    AJ::error::IErrorHandler& errHandler;
    alignas(CACHE_LINE_SIZE) std::atomic<bool> reading;

    AudioData() = default;

    AudioData(size_t size, int chans, AJ::error::IErrorHandler& handler) :
        errHandler(handler) {
            
        buffer = std::make_shared<AJ::utils::RingBuffer>(size, chans, handler);
        reading.store(false, std::memory_order_relaxed);
    }
};


class Recorder {
    AudioMetaData mAudioInfo;
    std::shared_ptr<AudioData> pAudioData;

    static int recordCallback(const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData);

public:

    Recorder(int samplerate, int channels, AJ::error::IErrorHandler& handler)
    : mAudioInfo(samplerate) {

        mAudioInfo.channels = channels;
        size_t size = mAudioInfo.mBufferSizePerChan * mAudioInfo.channels;

        pAudioData = std::make_shared<AudioData>(size, mAudioInfo.channels, handler);
    }

    bool record(AJ::error::IErrorHandler& errHandler, AJ::io::IEventHandler& evHandler);
};    
}