#pragma once
#include "error_handler.h"
#include "core/constants.h"

#include <cstdint>
#include <atomic>
#include <memory>
#include <cmath>
#include <cstring>

namespace AJ::utils {

/**
 * @class RingBuffer
 * @brief Lock-free single-producer, single-consumer (SPSC) ring buffer for real-time audio.
 *
 * This class provides a lock-free ring buffer implementation optimized for real-time audio use cases.
 *
 * @warning After construction, the caller **must check** if the buffer 
 *          is valid by calling `isValid()`. 
 *          - If `isValid()` returns false, the buffer cannot be used for any 
 *            read/write operations and the error handler will have already 
 *            been notified of the failure.
 * 
 *          - All other public member functions assume the buffer is valid 
 *            and will not perform additional checks.
 * 
 * ## Design:
 * - Buffer size is automatically rounded up to the next power of two for efficient masking.
 * - Supports mono or stereo channels (1 or 2).
 * - Provides methods for writing/reading single frames or bulk interleaved frames.
 * - lcok-free; safe for use in audio callbacks.
 * - Uses memory barriers (`std::memory_order_*`) to ensure correctness across threads.
 * - Uses Acquire-Release Pattern.
 * 
 * ## Usage:
 * - Writer thread: audio callback (`writeFrame`, `writeFrames`)
 * - Reader thread: disk/network worker (`readFrame`, `readFrames`)
 * - Single-producer, single-consumer only (not safe for multiple producers or consumers).
 *
 * ## Example:
 * @code
 * AJ::error::MyErrorHandler handler;
 * AJ::utils::RingBuffer rb(4096, 2, handler); // stereo, 4096 frames
 *
 * if(!rb.isValid()){
 *     //* don't continue.
 * }
 * 
 * float frame[2] = {0.5f, -0.3f};
 * rb.writeFrame(frame);
 *
 * float out[2];
 * if (rb.readFrame(out)) {
 *     //* out now contains the frame
 * }
 * @endcode
 */

class RingBuffer {
private:
    /**
     * @brief Write pointer index in the buffer (atomic).
     *
     * Updated by the producer (writer). Marked atomic to ensure visibility
     * across threads without locks.
     * 
     * aligned to prevent destructive interference (false-sharing),
     * which could cause cache line bouncing across cpu cores.
     */
    alignas(CACHE_LINE_SIZE) std::atomic<size_t> mWriteIndex{0};

    /**
     * @brief Read pointer index in the buffer (atomic).
     *
     * Updated by the consumer (reader). Marked atomic to ensure visibility
     * across threads without locks.
     * 
     * aligned to prevent destructive interference (false-sharing),
     * which could cause cache line bouncing across cpu cores.
     */
    alignas(CACHE_LINE_SIZE) std::atomic<size_t> mReadIndex{0};

    /**
     * @brief Raw pointer to the allocated buffer memory (float samples).
     *
     * Memory layout: interleaved channels (e.g., [L,R,L,R,...] for stereo).
     */
    alignas(CACHE_LINE_SIZE) float *mBuffer;


    /**
     * @brief Total buffer size (number of samples).
     *
     * Always a power of two for efficient modulo masking.
     */
    size_t mSize{0};

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
     * @brief flag to indicate whether the buffer is full or not
     * updated properly by the writer thread (writeFrame(s)) and the reader thread (readFrame(s)).
     */
    alignas(CACHE_LINE_SIZE) std::atomic<bool> mFullFlag;

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
    size_t getWriteSpace(size_t currentWrite){
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
    size_t getReadAvailable(size_t currentWrite, size_t currentRead){
        if(mFullFlag.load(std::memory_order_acquire)){
            return mSize;
        }

        return (currentWrite - currentRead) & mMask;
    }

    /**
     * @brief Free the buffer and reset indices.
     *
     * Cleans up all resources owned by the buffer. After calling this, the buffer
     * becomes invalid and must be reinitialized before reuse.
     */
    void cleanup(){
        mValid = false;
        mWriteIndex.store(0, std::memory_order_seq_cst);
        mReadIndex.store(0, std::memory_order_seq_cst);
        mSize = 0;
        mMask = 0;

        if(mBuffer){
            std::free(mBuffer);
            mBuffer = nullptr;
        }
    }

public: 
    /**
     * @brief Construct a lock-free ring buffer
     * 
     * @param size Desired buffer size per channel (will be rounded up to power of 2)
     * @param channels Number of audio channels (1=mono, 2=stereo)
     * @param handler Error handler for handling issues
     * 
     * @warning After construction, the caller **must check** if the buffer 
     *          is valid by calling `isValid()`. 
     *          - If `isValid()` returns false, the buffer cannot be used for any 
     *            read/write operations and the error handler will have already 
     *            been notified of the failure.
     * 
     *          - All other public member functions assume the buffer is valid 
     *            and will not perform additional checks.
     * 
     * Error Conditions:
     * - Returns an error if the number of channels is less than 1 or greater than 2.
     * - Returns an error if size equals 0.
     * - Returns an error if memory allocation fails.  
     *   In case of allocation failure, all previously allocated resources are cleaned up.
     * 
     * Design Notes:
     * - Buffer size is automatically rounded to next power of 2 for efficiency
     * - All memory is allocated during construction (no allocation during the reading / writing)
     * - Buffers are initialized to zero.
     * - Memory is 32-byte aligned for SIMD operations.
     */
    RingBuffer(size_t size, uint8_t channels, AJ::error::IErrorHandler& handler){
        mValid = false;
        mFullFlag.store(false, std::memory_order_relaxed);

        if(size == 0){
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

        size = next_power_of_2(size) * channels;
        mSize = size;
        mMask = size - 1;

        //* init the buffers.
        mBuffer = static_cast<float *>(
            std::aligned_alloc(32, size * sizeof(float)) //? 32 alognment for SIMD operations.
        );

        if(!mBuffer){
            const std::string message = std::bad_alloc().what();
            handler.onError(AJ::error::Error::ResourceAllocationFailed, message);
            cleanup();
            return;
        }

        //* init the buffer with 0 values.
        std::memset(mBuffer, 0, sizeof(float) * size);
        mValid = true;
    }

    /**
     * @brief Destructor - clean up all allocated memory
     * 
     * @note Safe to call even if buffer was never valid (will just no-op).
     */
    ~RingBuffer(){
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
     * @brief Write a single frame (all channels) to the buffer.
     *
     * @param samples Pointer to sample values, one per channel (size = mChannels).
     * 
     * @return true if the frame was written successfully, false if:
     *   - Buffer is full,
     *   - samples pointer is nullptr.
     *
     * This is the primary method for writing from the real-time audio callback.
     * It is wait-free and will never block.
     */
    bool writeFrame(const float *samples) noexcept;

    /**
     * @brief Write multiple frames to the buffer
     * 
     * @param input Pointer to interleaved audio data (ch0_sample0, ch1_sample0, ch0_sample1, ...)
     * @param frame_count Number of frames to write.
     * 
     * @return Number of frames actually written
     * 
     * @note the code output will break if the input buffer is not with the correct chunnel number and frame count.
     * 
     * This method handles bulk writes efficiently.
     * It's wait-free and will never block.
     */
    size_t writeFrames(const float* input, const size_t frame_count) noexcept;

    /**
     * @brief Read a single frame from the buffer
     * 
     * @param output Buffer to store sample values, one per channel     * 
     * 
     * @return true if read succeeded, false if output is nullptr
     * Or the mBuffer is empty. 
     * 
     * This is the primary read method for real-time audio processing.
     * It's wait-free and will never block.
     */
    bool readFrame(float *output) noexcept;

    /**
     * @brief Read multiple frames into interleaved format
     * 
     * @param output Buffer for interleaved audio data
     * @param frames_count Maximum number of frames to read
     * 
     * @return Number of frames actually read
     * 
     * This method is wait-free and will never block.
     */
    size_t readFrames(float *output, const size_t frames_count) noexcept;

    /**
     * @brief Get the total size (capacity) of the buffer in frames.
     * @return Number of frames (not samples).
     */
    size_t frameCapacity() const noexcept {
        return mSize / mChannels;
    }

    /**
     * @brief Get the total size (capacity) of the buffer in samples.
     * @return Number of samples (not frames).
     */
    size_t samplesCapacity() const noexcept {
        return mSize;
    }

    /**
     * @brief Get the number of channels stored per frame.
     * @return Channel count (e.g., 1 = mono, 2 = stereo).
     */
    size_t channels() const noexcept {
        return mChannels;
    }
    
};

} // namespace AJ::utils
