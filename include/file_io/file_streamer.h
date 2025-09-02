#pragma once 
#include "file_io/file_utils.h"
#include "core/buffer_pool.h"
#include "core/types.h"

#include <atomic>

namespace AJ::io::file_streamer {

class FileStreamer {
private:

    // Multithreading members.
    std::shared_ptr<AJ::utils::Queue> pQueue;
    std::shared_ptr<AJ::utils::BufferPool> pBufferPool;
    std::shared_ptr<std::atomic<bool>> pStopFlag;

    std::shared_ptr<AudioInfo> pReadInfo;
    std::shared_ptr<AudioWriteInfo> pWriteInfo;

    StreamingInfo mStreamingInfo;
public:

    FileStreamer(std::shared_ptr<AJ::utils::Queue> queue, std::shared_ptr<AJ::utils::BufferPool> pool,
        std::shared_ptr<std::atomic<bool>> stopFlag, AJ::FileStreamingTypes streaming_type): 
        pQueue(queue), pBufferPool(pool), pStopFlag(stopFlag), pReadInfo(nullptr), pWriteInfo(nullptr) {

        mStreamingInfo.type = streaming_type;
        
        std::string streaming_dir = SESSION_DIR + "/";
        switch (streaming_type)
        {
        case FileStreamingTypes::recording:
            streaming_dir += "records/";
            break;
        
        default:
            break;
        }

        // if the directory exists it will just do no operation
        AJ::utils::FileUtils::make_directory(streaming_dir);
        
        mStreamingInfo.directory = streaming_dir;
    }

    bool setReadInfo(const AudioInfo& info, AJ::error::IErrorHandler& handler);

    bool setWriteInfo(const AudioWriteInfo& info, AJ::error::IErrorHandler& handler);

    bool write(AJ::error::IErrorHandler& handler);

    bool read(AJ::error::IErrorHandler& handler);
};

}