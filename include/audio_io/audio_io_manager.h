#pragma once 

#include<memory>
#include "record.h"
#include "core/engine_resources.h"

namespace AJ::io::audio_io_manager {

/**
 * @brief Struct holding handlers required for recording.
 *
 * Provides references to the error handler and event handler 
 * that will be injected into the recording process.
 */
struct RecordHandlers {
    AJ::error::IErrorHandler& mRecordErrHandler;
    AJ::utils::IEventHandler& mRecordHandler;

    /**
     * @brief Construct a RecordHandlers object.
     *
     * @param recordErrHandler Reference to the error handler.
     * @param recordHandler Reference to the event handler.
     */
    RecordHandlers(AJ::error::IErrorHandler& recordErrHandler, AJ::utils::IEventHandler& recordHandler):
        mRecordErrHandler(recordErrHandler), mRecordHandler(recordHandler){}

};

/**
 * @brief Struct holding handlers required for playback.
 *
 * Provides references to the error handler and event handler 
 * that will be injected into the playback process.
 */
struct PlayHandlers {
    AJ::error::IErrorHandler& mPlayErrHandler;
    AJ::utils::IEventHandler& mPlayHandler;

    /**
     * @brief Construct a PlayHandlers object.
     *
     * @param playErrHandler Reference to the error handler.
     * @param playHandler Reference to the event handler.
     */
    PlayHandlers(AJ::error::IErrorHandler& playErrHandler, AJ::utils::IEventHandler& playHandler):
        mPlayErrHandler(playErrHandler), mPlayHandler(playHandler){}
};

/**
 * @brief Manages audio I/O operations such as recording and playback.
 *
 * The AudioIOManager ties together engine resources (thread pool, buffer pool, queues)
 * with higher-level recording and playback logic. Handlers for errors and events
 * are injected using dependency injection.
 *
 * Responsibilities:
 * - Validate and manage session directory.
 * - Initialize and manage the recorder with engine resources.
 * - Provide entry points for recording and playback.
 */
class AudioIOManager{
private:
    /**
     * @brief Path to the session directory.
     *
     * Used to store session-related files (e.g., audio recordings).
     * Validated before being used.
     */
    std::string mSessionDirectory;

    /**
     * @brief Recorder instance responsible for capturing audio.
     *
     * Created and initialized with engine resources and handlers.
     */
    std::shared_ptr<io::record::Recorder> pRecorder;

    /**
     * @brief Reference to the error handler for recording.
     *
     * Used to report errors that occur during recording operations.
     * Injected via dependency injection.
     */
    AJ::error::IErrorHandler& mRecordErrHandler;

    /**
     * @brief Reference to the event handler for recording.
     *
     * Handles interaction logic (e.g., console UI, GUI updates) during recording.
     * Injected via dependency injection.
     */
    AJ::utils::IEventHandler& mRecordHandler;

    /**
     * @brief Shared engine-wide resources.
     *
     * Provides access to thread pool, buffer pools, and queues
     * required for audio I/O operations.
     */
    std::shared_ptr<AJ::EngineResources> pEngineResources;

    /**
     * @brief Lock-free stop flag shared across components.
     *
     * Used to signal the termination of recording or playback.
     * Event handlers are responsible for setting this flag.
     */
    LFControlFlagPtr pStopFlag;

    /**
     * @brief Indicates whether the manager was successfully initialized.
     *
     * Set to true if all resources are valid and ready for use,
     * otherwise false.
     */
    bool mValid;

public:

    /**
     * @brief Construct an AudioIOManager instance.
     *
     * Initializes the manager with engine resources, session directory,
     * and user-provided handlers for recording and playback.
     *
     * @note should call `isValid()` immediately after construction to verify
     * that all resources (thread pool, queues, buffer pools, recorder, etc.)
     * were initialized correctly. 
     * 
     * @param engineResources Shared pointer to engine-wide resources.
     * @param session_directory Directory for storing audio session data.
     * @param record_handlers Handlers for recording (error + event).
     * @param play_handlers Handlers for playback (error + event).
     */
    AudioIOManager(std::shared_ptr<AJ::EngineResources> engineResources, std::string& session_directory,
        RecordHandlers& record_handlers, PlayHandlers& play_handlers) : pEngineResources(engineResources), 
        mRecordErrHandler(record_handlers.mRecordErrHandler),mRecordHandler(record_handlers.mRecordHandler){
        
        mValid = false;

        if(!AJ::utils::FileUtils::valid_directory(session_directory)){
            return;  
        }

        mSessionDirectory = session_directory;

        //* init recorder ptr.
        AJ::io::record::InitRecordInfo recorder_info;
        recorder_info.mChannels = 2;
        recorder_info.mSamplerate = 44100;
        recorder_info.mSessionDirectory = mSessionDirectory;

        if(!pEngineResources){
            return;
        }
        
        if(!pEngineResources->queueStereo() || !pEngineResources->bufferPoolStereo() || !pEngineResources->threadPool()){
            return;
        }
    
        if(!pEngineResources->queueStereo()->isValid() || !pEngineResources->bufferPoolStereo()->isValid()){
            return;
        }

        pStopFlag = std::make_shared<LFControlFlag>();
        pStopFlag->flag.store(false, std::memory_order_release);

        recorder_info.pQueue = pEngineResources->queueStereo();
        recorder_info.pBufferPool = pEngineResources->bufferPoolStereo();
        recorder_info.pThreadPool = pEngineResources->threadPool();
        recorder_info.pStopFlag = pStopFlag;
        
        pRecorder = std::make_shared<record::Recorder>(recorder_info, record_handlers.mRecordErrHandler);
        
        mValid = true;
    }

    /**
     * @brief Check whether the AudioIOManager was successfully initialized.
     *
     * This should be called immediately after construction to verify
     * that all resources (thread pool, queues, buffer pools, recorder, etc.)
     * were initialized correctly. 
     *
     * @return True if the manager is valid and ready for use, false otherwise.
     */
    bool isValid() const {
        return mValid;
    }


    /**
     * @brief Start the recording process.
     *
     * Delegates recording to the underlying Recorder instance.  
     * Recording will only start if the manager is valid (all resources initialized correctly).
     *
     * @return True if recording started successfully, false otherwise.
     */
    bool record(){
        if(!mValid){
            return false;
        }

        return pRecorder->record(mRecordHandler);
    }

    /**
     * @brief Start the playback process.
     *
     * Not yet implemented.
     *
     * @return True if playback started successfully, false otherwise.
     */
    bool play(){
        return false;
    }
};

}