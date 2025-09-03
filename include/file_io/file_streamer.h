#pragma once 
#include "file_io/file_utils.h"
#include "core/buffer_pool.h"
#include "core/types.h"
#include "core/error_handler.h"

#include <atomic>
#include <sndfile.h>

namespace AJ::io::file_streamer {

/**
 * @class FileStreamer
 * @brief Handles file I/O for audio streaming (recording/playback).
 *
 * The FileStreamer is designed to run in its own thread as part of the
 * recording or playback pipeline. It coordinates between:
 * - A lock-free `Queue` (producer-consumer buffer handoff).
 * - A `BufferPool` (recycling memory blocks for reuse).
 * - libsndfile (`SNDFILE`) for reading/writing audio files.
 *
 * - A stop flag (`pStopFlag`) is used to signal the writing thread to finish and flush any remaining buffers.
 *
 * ## Error Handling
 * - All runtime errors (invalid samplerate, failed file open/write/close, etc.)
 *   are reported via the provided `IErrorHandler`.
 * - Functions return `false` if initialization or I/O fails.
 */
class FileStreamer {
private:
    /**
     * @brief Queue of audio buffers (producer-consumer handoff).
     */
    std::shared_ptr<AJ::utils::Queue> pQueue;

    /**
     * @brief Pool of reusable buffers to avoid reallocations.
     */
    std::shared_ptr<AJ::utils::BufferPool> pBufferPool;

    /**
     * @brief Atomic flag used to stop the writer/reader thread.
     */
    std::shared_ptr<std::atomic<bool>> pStopFlag;

    /**
     * @brief Audio file information for reading (not implemented yet).
     */
    std::shared_ptr<AudioInfo> pReadInfo;

    /**
     * @brief Audio file information for writing (channels, samplerate, path).
     */
    std::shared_ptr<AudioWriteInfo> pWriteInfo;

    /**
     * @brief Streaming mode, directory information and file name.
     */
    StreamingInfo mStreamingInfo;

    /**
     * @brief Root session directory.
     */
    std::string mSessionDir;

private:
    /**
     * @brief Fill an `SF_INFO` struct with the current write settings.
     * @param info Reference to SF_INFO.
     */
    void set_file_info(SF_INFO& info);

    /**
     * @brief Close a SNDFILE file.
     * 
     * @param file Pointer to an opened SNDFILE.
     * @param return_val Value to return on success (used internally).
     * @param handler Error handler for reporting issues.
     * 
     * @return `return_val` if closed successfully, `false` otherwise.
     */
    bool close_file(SNDFILE *file, bool return_val, AJ::error::IErrorHandler &handler);

    /**
     * @brief Write an interleaved buffer to disk using libsndfile.
     * 
     * @param file Pointer to an opened SNDFILE.
     * @param buffer Audio buffer to write.
     * @param handler Error handler for reporting write failures.
     */
    void writeInterleaved(SNDFILE* file, AJ::utils::Buffer* buffer, AJ::error::IErrorHandler& handler);

public:
    /**
     * @brief Construct a new FileStreamer instance.
     *
     * @param queue Queue of buffers.
     * @param pool Buffer pool used for buffer recycling.
     * @param stopFlag Atomic flag to control thread stop.
     * @param streaming_type Streaming mode (recording, playback, etc.).
     * @param sessionDir Directory where files will be created/read.
     *
     * Creates a subdirectory under sessionDir depending on the streaming type
     * (e.g., `<sessionDir>/records/` for recording).
     */
    FileStreamer(std::shared_ptr<AJ::utils::Queue> queue,
        std::shared_ptr<AJ::utils::BufferPool> pool, std::shared_ptr<std::atomic<bool>> stopFlag,
        AJ::FileStreamingTypes streaming_type, std::string sessionDir):
        pQueue(queue), pBufferPool(pool), pStopFlag(stopFlag), mSessionDir(sessionDir), pReadInfo(nullptr), pWriteInfo(nullptr){
            
            mStreamingInfo.type = streaming_type; std::string streaming_dir = mSessionDir + "/";
            
            switch (streaming_type) { 
                case FileStreamingTypes::recording: 
                    streaming_dir += "records/"; break; 
                default: break; 
            }    
        
        // if the directory exists it will just do no operation
        AJ::utils::FileUtils::make_directory(streaming_dir); 
        mStreamingInfo.directory = streaming_dir; 

        std::string ext = "wav";
        mStreamingInfo.name = AJ::utils::FileUtils::generate_file_name(streaming_type, ext); 
    }

    /**
     * @brief Configure audio parameters for reading.
     * 
     * @param info Read settings (samplerate, channels).
     * @param handler Error handler.
     * 
     * @return `true` if valid, `false` otherwise.
     * 
     * @note Currently not implemented.
     */
    bool setReadInfo(const AudioInfo& info, AJ::error::IErrorHandler& handler);

    /**
     * @brief Configure audio parameters for writing.
     * 
     * @param info Write settings (samplerate, channels, filename).
     * @param handler Error handler.
     * 
     * @return `true` if configuration is valid and stored, `false` otherwise.
     * 
     * @note Only mono and stereo are supported. Samplerate must be one of:
     *       {8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000}.
     */
    bool setWriteInfo(const AudioWriteInfo& info, AJ::error::IErrorHandler& handler);

    /**
     * @brief Run the write loop (blocking).
     *
     * Opens the target file and writes audio buffers from the queue until
     * the stop flag is set. After stopping, any remaining buffers in the queue
     * are flushed to disk before the file is closed.
     *
     * @param handler Error handler.
     * @return `true` if file written successfully, `false` on error.
     */
    bool write(AJ::error::IErrorHandler& handler);

    /**
     * @brief Run the read loop (blocking).
     *
     * Opens the target file and streams audio buffers into the queue.
     * Intended for playback or analysis.
     *
     * @param handler Error handler.
     * 
     * @return `true` if file read successfully, `false` on error.
     * 
     * @note Not implemented yet.
     */
    bool read(AJ::error::IErrorHandler& handler);
};

}
