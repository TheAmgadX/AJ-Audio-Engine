#pragma once
#include <memory>

#include "core/buffer_pool.h"
#include "core/thread_pool.h"
#include "core/error_handler.h"

namespace AJ {

/**
 * @class EngineResources
 * @brief Central container that manages and provides access to core engine resources.
 *
 * The EngineResources class encapsulates shared resources required by the audio engine,
 * such as the thread pool, buffer pools, and queues. This ensures that subsystems
 * (recording, playback, processing) can reuse the same resources in a coordinated manner.
 *
 * ### Resources:
 * - **Thread Pool**: Provides a reusable set of threads to avoid repeated allocation and deallocation overhead.
 * 
 * - **Buffer Pools**: Pre-allocated memory blocks for mono or stereo audio sample storage.
 * 
 * - **Queues**: Lock-free queues for passing audio buffers between producers (e.g., driver callbacks) 
 *   and consumers (e.g., file writers, processors).
 */
class EngineResources {
private:
    std::shared_ptr<AJ::utils::ThreadPool> pThreadPool;       ///< Shared thread pool.

    std::shared_ptr<AJ::utils::BufferPool> pBufferPoolMono;   ///< Buffer pool for mono audio data.
    std::shared_ptr<AJ::utils::Queue> pQueueMono;             ///< Queue for mono audio streaming.

    std::shared_ptr<AJ::utils::BufferPool> pBufferPoolStereo; ///< Buffer pool for stereo audio data.
    std::shared_ptr<AJ::utils::Queue> pQueueStereo;           ///< Queue for stereo audio streaming.

public:
    /**
     * @brief Construct engine resources with default pools and queues.
     *
     * Initializes the following resources:
     * - Thread pool (at least one thread guaranteed).
     * - Mono buffer pool and queue.
     * - Stereo buffer pool and queue.
     *
     * @param handler Reference to an error handler used to report initialization issues.
     */
    EngineResources(AJ::error::IErrorHandler& handler){
        pThreadPool = std::make_shared<utils::ThreadPool>();
        pBufferPoolMono = std::make_shared<utils::BufferPool>(handler, 1024, 1024, 1);
        pBufferPoolStereo = std::make_shared<utils::BufferPool>(handler);

        pQueueMono = std::make_shared<utils::Queue>(true, 1024, 1024, 1, handler);
        pQueueStereo = std::make_shared<utils::Queue>(true, 1024, 1024, 2, handler);
    }

    /**
     * @brief Get the shared thread pool.
     * @return std::shared_ptr to ThreadPool.
     */
    std::shared_ptr<AJ::utils::ThreadPool> threadPool() const {
        return pThreadPool;
    }

    /**
     * @brief Get the mono buffer pool.
     * @return std::shared_ptr to BufferPool for mono audio.
     */
    std::shared_ptr<AJ::utils::BufferPool> bufferPoolMono() const {
        return pBufferPoolMono;
    }

    /**
     * @brief Get the mono queue.
     * @return std::shared_ptr to Queue for mono audio streaming.
     */
    std::shared_ptr<AJ::utils::Queue> queueMono() const {
        return pQueueMono;
    }

    /**
     * @brief Get the stereo buffer pool.
     * @return std::shared_ptr to BufferPool for stereo audio.
     */
    std::shared_ptr<AJ::utils::BufferPool> bufferPoolStereo() const {
        return pBufferPoolStereo;
    }

    /**
     * @brief Get the stereo queue.
     * @return std::shared_ptr to Queue for stereo audio streaming.
     */
    std::shared_ptr<AJ::utils::Queue> queueStereo() const {
        return pQueueStereo;
    }
};

}
