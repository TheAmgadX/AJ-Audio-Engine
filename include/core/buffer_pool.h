#pragma once 
#include "types.h"
#include "error_handler.h"

#include <vector>
#include <cstdint>
#include <atomic>
#include <memory>
#include <cmath>
#include <cstring>

namespace AJ::utils {

/**
 * @class Queue
 * @brief Lock-free single-producer, single-consumer (SPSC) queue Buffers may either be pre-allocated internally 
 * (if constructed with empty = false), or managed externally (if constructed with empty = true).
 * 
 * In empty mode, the queue starts with no buffers. The producer must push buffers before the consumer can pop.
 * In full mode, the queue pre-allocates and zero-initializes all buffers; the consumer can immediately pop them.
 *
 * This class implements a fixed-size ring queue designed for real-time audio processing.
 * Instead of managing individual buffers, the queue holds pre-allocated buffers of frames.
 * This design avoids dynamic allocation in real-time threads and ensures predictable latency.
 *
 * ## Design:
 * - Queue capacity (`queue_size`) determines how many buffers can be enqueued at once.
 * - Each buffer has a fixed size in frames (`buffer_size`), multiplied internally by the
 *   number of channels to determine the sample count.
 * - Buffers are pre-allocated on construction and stored in the queue.
 * - The queue is wait-free, non-blocking, and safe for SPSC usage (one producer, one consumer).
 * - Memory is aligned to 32 bytes to allow SIMD optimizations.
 * - Acquire/Release semantics (`std::memory_order_*`) ensure correct synchronization.
 *
 * @warning After construction, the caller **must check** if the Queue 
 *          is valid by calling `isValid()`. 
 *          - If `isValid()` returns false, the buffer cannot be used for any 
 *            read/write operations and the error handler will have already 
 *            been notified of the failure.
 * 
 *          - All other public member functions assume the buffer is valid 
 *            and will not perform additional checks.
 * 
 *          - The behavior of push() and pop() depends on the empty flag provided at construction.
 * 
 *          - In empty mode the queue doesn't own the buffers so it doesn't free them,
 *            you must return them back to the owner!
 * 
 * ## Usage:
 * - Producer thread (e.g. reader thread):
 *   - Fills a buffer from the pool.
 *   - Calls `push()` to enqueue it for the consumer.
 *
 * - Consumer thread (e.g. disk writer):
 *   - Calls `pop()` to dequeue the next buffer pointer.
 *   - Processes/writes it to disk.
 *
 * ## Example:
 * @code
 * AJ::error::MyErrorHandler handler;
 *
 * //* -------------------------------------------------
 * //* Example 1: Queue in empty mode (external buffers)
 * //* -------------------------------------------------
 *
 * //* Create queue with capacity for 128 pointers,
 * //* each buffer is expected to hold 1024 frames (stereo).
 * //* No internal allocation is done.
 * AJ::utils::Queue q_empty(true, 128, 1024, 2, handler);
 *
 * if (!q_empty.isValid()) {
 *     //* don't continue.
 * }
 *
 * //* Producer thread: push external buffer into queue
 * float* extBuf = externalAllocator();   // user-allocated
 * q_empty.push(extBuf);
 *
 * //* Consumer thread: pop buffer, process, and free externally
 * float* out1 = q_empty.pop();
 * if (out1) {
 *     //* Write 'out1' to disk...
 *     externalFree(out1);
 * }
 *
 *
 * //* -------------------------------------------------
 * //* Example 2: Queue in full mode (buffer pool style)
 * //* -------------------------------------------------
 *
 * //* Create queue with 128 *pre-allocated* buffers
 * AJ::utils::Queue q_full(false, 128, 1024, 2, handler);
 *
 * if (!q_full.isValid()) {
 *     //* don't continue.
 * }
 *
 * //* Producer thread: pop pre-allocated buffer, fill with samples
 * float* buf = q_full.pop();
 * if (buf) {
 *     //* Fill buf with audio samples...
 *     q_full.push(buf);
 * }
 *
 * //* Consumer thread: pop buffer and recycle it back into the pool
 * float* out2 = q_full.pop();
 * if (out2) {
 *     //* Write 'out2' to disk...
 *     q_full.push(out2);
 * }
 * @endcode
 *
 * ## Error Handling:
 * - Construction fails if:
 *   - Queue size = 0 or buffer size = 0.
 *   - Channels < 1 or > 2.
 *   - Memory allocation fails (error handler notified).
 *
 * ## Notes:
 * - Only supports single producer and single consumer (SPSC).
 * - No runtime allocations; everything is pre-allocated up front.
 * - Safe for use in real-time audio contexts (e.g., DAWs, game engines).
 */

class Queue {
private:
    /**
     * @brief Write pointer index in the buffer (atomic).
     *
     * Updated by the producer (writer). Marked atomic to ensure visibility
     * across threads without locks.
     */
    alignas(CACHE_LINE_SIZE) std::atomic<size_t> mWriteIndex{0};

    
    /**
     * @brief Read pointer index in the buffer (atomic).
     *
     * Updated by the consumer (reader). Marked atomic to ensure visibility
     * across threads without locks.
     */
    alignas(CACHE_LINE_SIZE) std::atomic<size_t> mReadIndex{0};

    /**
     * @brief flag to indicate whether the buffer is full or not
     * updated by the writer thread (push) and the reader thread (pop).
     * used in the empty mode only.
     */
    alignas(CACHE_LINE_SIZE) std::atomic<bool> mFullFlag;

    /**
     * @brief Raw pointer to the allocated buffer memory (float samples).
     *
     * Memory layout: interleaved channels (e.g., [L,R,L,R,...] for stereo).
     */
    alignas(CACHE_LINE_SIZE) std::vector<float *>mQueue;


    /**
     * @brief Total buffer size (number of samples).
     *
     * Always a power of two for efficient modulo masking.
     */
    size_t mBufferSize{0};

    /**
     * @brief Total Buffers in the queue
     * 
     * Always a power of two for efficient modulo masking.
     */
    size_t mQueueSize{0};

    /**
     * @brief Mask used for fast wrapping of indices (mSize - 1).
     */
    size_t mMask{0};

    /**
     * @brief Number of channels in the buffer (1 = mono, 2 = stereo).
     */
    uint8_t mChannels{1};

    /**
     * @brief Whether the buffer is initialized and ready for use.
     */
    bool mValid{false};

    /**
     * @brief Indicates the initial state of the queue.
     *
     * - If true, the queue starts empty (no buffers pre-initialized). 
     *   Push operations must occur before buffers can be popped.
     *
     * - If false, the queue starts full (buffers are pre-initialized and ready to be consumed). 
     *   Pop operations can proceed immediately.
     * 
     * @warning In empty mode the queue doesn't own the buffers so it doesn't free them.
     *   you must return them back to the owner!
     */
    bool mEmptyQueue{false};

private:
    /**
     * @brief Round up to next power of 2
     */
    static constexpr size_t next_power_of_2(size_t n) {
        if (n <= 1) return 1;
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n |= n >> 32;
        return n + 1;
    }
    
    /**
     * @brief Get available space for writing (from writer's perspective)
     */
    size_t getFreeSpace(size_t currentWrite){
        size_t currentRead = mReadIndex.load(std::memory_order_acquire);

        if(mFullFlag.load(std::memory_order_acquire)){
            return 0;
        }

        /*
         * this different calculation to make sure if the write is 0 and the read is 0 and 
         * the buffer is not full, the calculation returns the full size.
         */
        return ((currentRead - currentWrite - 1) & mMask) + 1;
    }

    /**
     * @brief Get available data for reading (from reader's perspective)
     */
    size_t getAvailableBuffers(size_t currentWrite, size_t currentRead){
        if(mFullFlag.load(std::memory_order_acquire)){
            return mQueueSize;
        }

        return (currentWrite - currentRead) & mMask;
    }

    /**
     * @brief Free the buffer and reset indices.
     *
     * Cleans up all resources owned by the buffer. After calling this, the buffer
     * becomes invalid and must be reinitialized before reuse.
     * 
     * @warning In empty mode the queue doesn't own the buffers so it doesn't free them.
     *   you must return them back to the owner! 
     */
    void cleanup(){
        mValid = false;
        mWriteIndex.store(0, std::memory_order_seq_cst);
        mReadIndex.store(0, std::memory_order_seq_cst);
        
        if(mEmptyQueue){
            mFullFlag.store(false, std::memory_order_seq_cst);
        } else {
            mFullFlag.store(true, std::memory_order_seq_cst);
        }
        
        mBufferSize = 0;
        mQueueSize = 0;
        mMask = 0;
        
        if(mEmptyQueue){
            mQueue.clear();
            return;
        }

        for(auto& buffer : mQueue){
            if(buffer){
                delete[] buffer;
                buffer = nullptr;
            }
        }

        mQueue.clear();
    }

    /**
     * @brief allocates buffers of the queue.
     */
    void allocBuffers(size_t queue_size, size_t buffer_size, AJ::error::IErrorHandler& handler){
        //* init the buffers.
        for(size_t i = 0; i < queue_size; ++i){
            mQueue[i] = new float[mBufferSize];

            if(!mQueue[i]){
                const std::string message = std::bad_alloc().what();
                handler.onError(AJ::error::Error::ResourceAllocationFailed, message);
                cleanup();
                return;
            }

            //* init the buffer with 0 values.
            std::memset(mQueue[i], 0, sizeof(float) * buffer_size);
        }

        mValid = true;
    }

public: 
    /**
     * @brief Construct a lock-free audio buffer queue.
     *
     * This constructor pre-allocates a fixed number of audio buffers 
     * (each aligned to 32 bytes for SIMD optimization) and arranges them 
     * in a lock-free ring queue. The queue provides pre-zeroed buffers for 
     * real-time audio processing tasks such as disk I/O or effect processing.
     *
     * @param empty Indicates the initial state of the queue. (empty or full)
     *        - If true, the queue starts empty (no buffers pre-initialized). 
     *           Push operations must occur before buffers can be popped.
     *
     *        - If false, the queue starts full (buffers are pre-initialized and ready to be consumed). 
     *           Pop operations can proceed immediately.     
     * 
     * @param queue_size Desired number of buffers in the queue.  
     *        - Rounded up to the next power of 2 internally for efficient masking.  
     *        - Must be greater than 0.
     *
     * @param buffer_size Desired size of each buffer per channel (in frames).  
     *        - Rounded up to the next power of 2.  
     *        - Multiplied by `channels` internally 
     *          to get the size in sampels (e.g., stereo doubles the storage).
     *
     * @param channels Number of audio channels.  
     *        - Supported values: `1` (mono), `2` (stereo).  
     *
     * @param handler Reference to an error handler for reporting allocation or 
     *        configuration errors.
     *
     * @warning After construction, the caller **must** call `isValid()` 
     *          before using the queue.  
     *          - If `isValid()` returns `false`, the queue is unusable and 
     *            the error handler has already been notified.  
     *          - All other public methods assume the queue is valid.
     *
     * ### Error Conditions
     * - If `queue_size == 0` or `buffer_size == 0`, construction fails.  
     * - If `channels` is not 1 or 2, construction fails.  
     * - If memory allocation fails at any point, construction fails and all 
     *   previously allocated resources are freed.
     *
     * ### Design Notes
     * - The queue is designed for **real-time audio** use cases (no runtime allocations).  
     * - Buffers are **32-byte aligned** to ensure compatibility with SIMD instructions.  
     * - All buffers are **zero-initialized**.  
     * - Internally uses power-of-2 sizes to enable efficient masking (`index & (size-1)`).
     *
     * Example usage:
     * @code
     * AJ::error::MyErrorHandler handler;
     * Queue q(false, 1024, 512, 2, handler);
     * if (!q.isValid()) {
     *     //* Handle error
     * }
     * @endcode
     */
    Queue(bool empty, size_t queue_size, size_t buffer_size, uint8_t channels, AJ::error::IErrorHandler& handler){
        mValid = false;
        mFullFlag.store(!empty, std::memory_order_relaxed);

        if(queue_size == 0 || buffer_size == 0){
            const std::string message = "Error: invalid buffer size.\n";
            handler.onError(AJ::error::Error::InvalidBufferSize, message);
            return;
        }
        
        if(channels > 2 || channels < 1){
            const std::string message = "Error: Unsupported channels number only support mono and stereo.\n";
            handler.onError(AJ::error::Error::InvalidChannelCount, message);
            return;
        }

        mChannels = channels;

        buffer_size = next_power_of_2(buffer_size) * channels;
        mBufferSize = buffer_size;

        queue_size = next_power_of_2(queue_size);
        mQueueSize = queue_size;
        mMask = queue_size - 1;

        mQueue.resize(queue_size, nullptr);

        if(empty){
            mValid = true;
            mEmptyQueue = true;
            return;
        }

        allocBuffers(queue_size, buffer_size, handler);
    }

    /**
     * @brief Destructor - clean up all allocated memory
     * 
     * @note Safe to call even if buffer was never valid (will just no-op).
     * 
     * @warning In empty mode the queue doesn't own the buffers so it doesn't free them.
     *   you must return them back to the owner! 
     */
    ~Queue(){
        cleanup();
    }

    /**
     * @brief Check if the buffer is valid (properly initialized).
     * must be used after initializing before any processing.
     * 
     * @return true if valid, false otherwise.
     */
    bool isValid() const noexcept {
        return mValid;
    }

    /**
     * @brief Attempt to push a new buffer into the queue.
     * 
     * @param buffer pointer to audio buffer.
     * 
     * @return `true` if the push succeeded (a free slot was available),  
     *         `false` if the queue is full and no slot could be reserved.
     * 
     * @note This function is non-blocking and lock-free.
     */
    bool push(float* buffer) noexcept;


    /**
     * @brief Pop the next available buffer from the queue.
     * 
     * This function retrieves a pointer to the buffer at the current 
     * read position and advances the read index.
     * 
     * @return Pointer to the buffer (`float*`) if data is available,  
     *         or `nullptr` if the queue is empty.
     * 
     * @note This function is non-blocking and lock-free.
     * 
     * @warning The caller must not hold onto the buffer pointer 
     *          indefinitely, since it will be reused by the producer.
     */
    float* pop() noexcept;

    /**
     * @brief Get the total size (capacity) of the Queue buffers in frames.
     * @return Number of frames (not samples).
     */
    size_t bufferFrameCapacity() const noexcept {
        return mBufferSize / mChannels;
    }

    /**
     * @brief Get the total size (capacity) of the Queue buffers in samples.
     * @return Number of samples (not frames).
     */
    size_t bufferSamplesCapacity() const noexcept {
        return mBufferSize;
    }

    /**
     * @brief Get the total size (capacity) of the Queue.
     * @return Number of buffers in the queue.
     */
    size_t queueSize() const noexcept {
        return mQueueSize;
    }

    /**
     * @brief Get the number of channels stored in Queue Buffers per frame.
     * @return Channel count (e.g., 1 = mono, 2 = stereo).
     */
    size_t channels() const noexcept {
        return mChannels;
    }
        
    /**
     * @brief the current size of the queue.
     * @note perform atomic operations (aquire loads) for both mWriteIndex, mReadIndex.
     * 
     * @return the current number of buffers in the queue.
     */
    size_t currentSize();
};

/**
 * @class BufferPool
 * @brief Manages a pool of pre-allocated audio buffers for producer-consumer workflows.
 * 
 * @note Internally, BufferPool creates a Queue in full mode (buffers pre-allocated and zero-initialized).
 * 
 * @see /include/core/buffer_pool.h -> Queue
 * 
 * The BufferPool provides a thread-safe mechanism for managing reusable audio buffers
 * in a multi-threaded environment. It wraps an internal fixed-size queue of buffers
 * and exposes push/pop and status-checking operations.
 * 
 * Typical usage involves:
 * - A producer thread (e.g., real-time audio callback) pushing filled buffers.
 * - A consumer thread (e.g., disk writer or DSP processor) popping buffers for further use.
 * 
 * Buffers are allocated once during construction and then reused, avoiding repeated 
 * allocations during real-time processing.
 */
class BufferPool {
private:
    std::shared_ptr<Queue> pBuffersQueue; ///< Underlying buffer queue managing the actual memory.

public:

    /**
     * @brief Construct a new BufferPool.
     * 
     * @param handler        Reference to error handler used for reporting allocation or runtime errors.
     * @param num_of_buffers Number of pre-allocated buffers in the pool (default = 1024).
     * @param buffer_frames  Number of float frames per buffer (default = 1024).
     * @param channels       Number of channels per buffer (1 = mono, 2 = stereo, etc., default = 2).
     * @param empty          Flag 
     * 
     * @note All buffers are allocated at construction time and reused for the pool’s lifetime.
     *       This avoids memory allocations in the real-time path.
     */
    BufferPool(AJ::error::IErrorHandler& handler,
               size_t num_of_buffers = 1024,
               size_t buffer_frames = 1024,
               uint8_t channels = 2) {
        pBuffersQueue = std::make_shared<Queue>(false, num_of_buffers, buffer_frames, channels, handler);
    }

    /**
     * @brief Push a buffer into the pool for later reuse.
     * 
     * @param handler Reference to error handler for reporting invalid buffers or queue overflow.
     *
     * @param buffer Pointer to a buffer previously obtained via `pop()`.
     * 
     * @return `true` if the buffer was successfully returned to the pool,  
     *         `false` if the pool is already full or the buffer is nullptr.
     * 
     * @note This function is typically called by a consumer thread after finishing 
     *       processing or writing the buffer to disk.
     */
    bool push(float* buffer, AJ::error::IErrorHandler& handler) {
        if(!buffer){
            const std::string message = "error: invalid buffer, buffer cannot be NULL.";
            handler.onError(AJ::error::Error::NullBufferPtr, message);
            return false;
        }

        if(!pBuffersQueue->push(buffer)){
            const std::string message = "error: queue is full.";
            handler.onError(AJ::error::Error::BufferOverflow, message);
            return false;
        }

        return true;
    }

    /**
     * @brief Pop a buffer from the pool for writing or processing.
     * 
     * @param handler Reference to error handler for reporting empty queue cases.
     * 
     * @return Pointer to a float buffer if available, or `nullptr` if the pool is empty.
     * 
     * @note This function is typically called by a producer thread
     *       to obtain an consumed or empty buffer for writing new audio samples.
     * 
     * @warning The buffer must eventually be returned via `push()` once processing is complete.
     */
    float* pop(AJ::error::IErrorHandler& handler) {
        float* buffer = pBuffersQueue->pop();
        
        if(!buffer){
            const std::string message = "error: queue is empty.";
            handler.onError(AJ::error::Error::EmptyBufferQueue, message);
            return nullptr;
        }

        return buffer;
    }

    /**
     * @brief Check whether the buffer pool is in a valid state.
     * 
     * @return `true` if the queue and buffers were successfully allocated  
     *         and are ready for use, `false` otherwise.
     */
    bool isValid() {
        return pBuffersQueue->isValid();
    }

    /**
     * @brief Get the total number of buffers managed by this pool.
     * 
     * @return Number of buffers pre-allocated in the pool.
     */
    int capacity() const {
        return pBuffersQueue->queueSize();
    }

    /**
     * @brief Get the number of channels per buffer.
     * 
     * @return The channel count (e.g., 1 = mono, 2 = stereo).
     */
    int channels() const {
        return pBuffersQueue->channels();
    }

    /**
     * @brief Get the size of each buffer in samples per channel.
     * 
     * @return Buffer size (number of float samples per channel).
     */
    int bufferSize() const {
        return pBuffersQueue->bufferSamplesCapacity();
    }

    /**
     * @brief the current size of the queue.
     * @note perform atomic operations (aquire loads) for both mWriteIndex, mReadIndex.
     * 
     * @return the current number of buffers in the queue.
     */
    size_t currentSize(){
        return pBuffersQueue->currentSize();
    }

};

}