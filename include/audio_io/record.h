#pragma once 
#include <memory>

#include "portaudio.h"

#include "core/types.h"
#include "core/error_handler.h"
#include "core/ring_buffer.h"
#include "core/buffer_pool.h"
#include "core/thread_pool.h"
#include "core/event_handler.h"

#include "file_io/file_streamer.h"

namespace AJ::io::record {

/**
 * @brief Initialization info required to set up a Recorder instance.
 *
 * This struct encapsulates all the configuration and resource dependencies
 * needed by the Recorder during setup, including sampling rate, channels,
 * session directory, and references to engine resources such as buffer pools,
 * queues, and thread pools.
 */
struct InitRecordInfo { 
    int mSamplerate;   ///< Recording sample rate in Hz.
    uint8_t mChannels; ///< Number of audio channels (1 = mono, 2 = stereo, etc.).
    std::string mSessionDirectory; ///< Directory where recorded session files will be stored.

    LFControlFlagPtr pStopFlag; ///< Lock-free stop flag used to control recording state.
    std::shared_ptr<AJ::utils::ThreadPool> pThreadPool; ///< Thread pool used for background tasks (e.g., disk writing).
    std::shared_ptr<AJ::utils::BufferPool> pBufferPool; ///< Buffer pool for audio data storage.
    std::shared_ptr<AJ::utils::Queue> pQueue; ///< Queue for transferring audio buffers between threads.
};

/**
 * @brief Holds runtime metadata for the audio stream being recorded.
 *
 * Contains stream parameters such as samplerate, channels, buffer sizes, and
 * PortAudio stream handle.
 */
class AudioMetaData {
public:
    int samplerate;                ///< Recording sample rate.
    uint8_t channels;              ///< Number of channels.
    size_t frames_per_buffer;      ///< Frames per buffer (derived from buffer size and channels).
    
    PaStream* stream;              ///< PortAudio stream pointer.

    size_t mBufferSizePerChan;     ///< Buffer size per channel.
    std::string mSessionDirectory; ///< Directory for storing session files.

    AudioMetaData(){}

    /**
     * @brief Construct metadata with a given sample rate.
     * @param rate The sample rate in Hz.
     */
    AudioMetaData(int rate): samplerate(rate) {
        mBufferSizePerChan = samplerate * BUFFER_SECONDS;    
    }
};

/**
 * @brief Runtime container for Recorder resources and state.
 *
 * Stores buffer pool, queue, stop flag, and error handler references for
 * managing audio data during port audio callback.
 */
class AudioData {
public:
    std::shared_ptr<AJ::utils::BufferPool> pBufferPool; ///< Buffer pool for managing audio buffers.
    std::shared_ptr<AJ::utils::Queue> pQueue;           ///< Queue for transferring buffers.
    LFControlFlagPtr pStopFlag;                         ///< Stop flag controlling recording lifecycle.
    AJ::error::IErrorHandler& errHandler;               ///< Error handler reference for reporting errors.

    AudioData() = default;

    /**
     * @brief Construct AudioData with required resources.
     */
    AudioData(std::shared_ptr<AJ::utils::BufferPool> pool,
              std::shared_ptr<AJ::utils::Queue> queue,
              LFControlFlagPtr stopFlag,
              AJ::error::IErrorHandler& handler) 
        : errHandler(handler) 
    {
        pBufferPool = pool;
        pQueue = queue;
        pStopFlag = stopFlag;
    }
};

/**
 * @brief Core class responsible for managing audio recording.
 *
 * The Recorder sets up audio metadata, manages buffers and queues,
 * initializes the PortAudio stream, and spawns background disk writer tasks
 * using the thread pool. Recording is stopped by toggling the stop flag
 * provided in InitRecordInfo.
 * 
 * @note this functionality requires at least two threads available in the thread pool,
 *       one for disk writer and one available to the Event Handler.
 */
class Recorder {
    AudioMetaData mAudioInfo;                   ///< Audio metadata for current recording session.
    std::shared_ptr<AudioData> pAudioData;      ///< Audio data and resource container.

    LFControlFlagPtr pStopFlag;                 ///< Stop flag for controlling recording lifecycle.
    std::shared_ptr<AJ::utils::ThreadPool> pThreadPool; ///< Thread pool for async tasks.

    std::shared_ptr<AJ::io::file_streamer::FileStreamer> pStreamer; ///< File streamer for writing audio to disk.

private:
    /**
     * @brief Initialize the recorder and prepare the audio stream.
     * 
     * @return True if initialization succeeds, false otherwise.
     */
    bool initRecorder();

    /**
     * @brief PortAudio callback for capturing input audio buffers.
     *
     * Called by PortAudio when new audio data is available.
     * The input data is copied into a buffer obtained from the buffer pool,
     * then pushed into the queue for consumption by the disk writer thread.
     */
    static int recordCallback(const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData);

    /**
     * @brief Background task for writing audio data to disk.
     * 
     * Retrieves a buffer from the queue when available,
     * writes its contents to disk, and then returns the buffer to the pool.
     */
    void diskWriter();
    
public:
    /**
     * @brief Construct a Recorder with initialization info and error handler.
     * 
     * @param info Initialization parameters (resources, samplerate, etc.).
     * @param handler Error handler for reporting failures.
     */
    Recorder(InitRecordInfo& info, AJ::error::IErrorHandler& handler){
        mAudioInfo = AudioMetaData(info.mSamplerate);
        mAudioInfo.channels = info.mChannels;
        mAudioInfo.mSessionDirectory = info.mSessionDirectory;

        size_t size = mAudioInfo.mBufferSizePerChan * mAudioInfo.channels;
        mAudioInfo.frames_per_buffer = size;

        pStopFlag = info.pStopFlag;
        pThreadPool = info.pThreadPool;

        pAudioData = std::make_shared<AudioData>(info.pBufferPool, info.pQueue, info.pStopFlag, handler);
        
        pStreamer = std::make_shared<AJ::io::file_streamer::FileStreamer>(
            info.pQueue, info.pBufferPool, info.pStopFlag,
            AJ::FileStreamingTypes::recording, info.mSessionDirectory
        );

        AJ::AudioWriteInfo write_info;
        write_info.channels = info.mChannels;
        write_info.samplerate = info.mSamplerate;

        pStreamer->setWriteInfo(write_info, handler);
    }

    /**
     * @brief Begin recording process with a given event handler.
     *
     * Starts the recording stream and invokes the event handler to handle
     * user interaction (e.g., console timer, GUI, etc.).
     *
     * At least one thread is guaranteed to be available in the thread pool
     * for the handler to use if needed. The handler is also responsible for
     * stopping the recording by setting the stop flag to true.
     *
     * @param evHandler Event handler used for handling UI/interaction logic.
     * 
     * @return True if recording starts successfully, false otherwise.
     */
    bool record(AJ::utils::IEventHandler& evHandler);
};    

}

