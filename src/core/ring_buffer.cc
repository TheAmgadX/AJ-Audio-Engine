#include "core/ring_buffer.h"

bool AJ::utils::RingBuffer::writeFrame(const float *samples) noexcept {
    if(!samples){
        return false;
    }
    
    size_t currentWrite = mWriteIndex.load(std::memory_order_relaxed);
    
    size_t space = getWriteSpace(currentWrite);

    if(space < mChannels) return false;

    for(int ch = 0; ch < mChannels; ++ch){
        mBuffer[currentWrite] = samples[ch];
        currentWrite = (currentWrite + 1) & mMask;
    }

    mWriteIndex.store(currentWrite, std::memory_order_release);

    if(space == mChannels){
        mFullFlag.store(true, std::memory_order_release);
    }

    return true;
}

size_t AJ::utils::RingBuffer::writeFrames(const float* input, const size_t frame_count) noexcept {
    if(!input) return 0;

    size_t currentWrite = mWriteIndex.load(std::memory_order_relaxed);
    
    size_t available_samples = getWriteSpace(currentWrite);

    if(available_samples == 0){
        return 0;
    }

    size_t available_frames = available_samples / mChannels;

    //* actual written samples count.
    size_t written_samples = frame_count * mChannels;

    //* actual written frames count.
    size_t written_frames;

    //* flag to determine whether we should update mFullFlag or not.
    bool full = false;

    if(available_samples <= written_samples){
        written_frames = available_frames;
        written_samples = available_samples;
        full = true;
    } else {
        written_frames = frame_count;
    }

    size_t first = std::min(mSize - currentWrite, written_samples);

    std::memcpy(&mBuffer[currentWrite], &input[0], first * sizeof(float));
    currentWrite = (currentWrite + first) & mMask;
    
    //* write the rest of the samples if exist
    if(first != written_samples){
        size_t second = written_samples - first;
        std::memcpy(&mBuffer[currentWrite], &input[first], second * sizeof(float));
        currentWrite = (currentWrite + second) & mMask;
    }

    mWriteIndex.store(currentWrite, std::memory_order_release);

    if(full){
        mFullFlag.store(true, std::memory_order_release);
    }

    return written_frames;
}

bool AJ::utils::RingBuffer::readFrame(float *output) noexcept {
    if(output == nullptr) return false;

    size_t currentWrite = mWriteIndex.load(std::memory_order_acquire);
    size_t currentRead = mReadIndex.load(std::memory_order_relaxed);

    size_t available_samples = getReadAvailable(currentWrite, currentRead);
    
    if(available_samples < mChannels || !output){
        return false;
    }

    for(uint8_t ch = 0; ch < mChannels; ++ch){
        output[ch] = mBuffer[currentRead];
        currentRead = (currentRead + 1) & mMask;
    }

    mReadIndex.store(currentRead, std::memory_order_release);

    //* update the full flag only if it was full before read operation.
    /*
    * p = full flag is true before read operation.
    * q = update full flag to be false.
    * p <=> q
    */
    if(available_samples == mSize){
        mFullFlag.store(false, std::memory_order_release);
    }

    return true;
}

size_t AJ::utils::RingBuffer::readFrames(float *output, const size_t frames_count) noexcept {
    if(output == nullptr || frames_count == 0) return 0;

    size_t currentWrite = mWriteIndex.load(std::memory_order_acquire);
    size_t currentRead = mReadIndex.load(std::memory_order_relaxed);
    
    size_t available_samples = getReadAvailable(currentWrite, currentRead);
    
    if(available_samples == 0 || !output) return 0;

    //* actual frames count that have been read.
    size_t read_frames = 0;

    //* actual samples count that have been read.
    size_t read_samples = frames_count * mChannels;

    if(available_samples < read_samples){
        read_samples = available_samples;
    }

    //* read data from mBuffer to output.
    /*
        * we have two cases:
            - case 1: read index < write index:
                read from read index to min(write index - 1, read_samples).
                there is no remaining samples in this case.

            - case 2: read index > write index:
                read from read index to first = min(end, read_samples).
                if there is remaining samples (first != read_samples): 
                    read from index 0 to second = min(write_index - 1, read_samples - first).

        * Note: some std::min() checks are not required based on the function flow, but it's done for more safety and clarity.
    */

    //* case 1:
    if(currentRead < currentWrite){
        size_t read_size = currentWrite - currentRead;
        read_size = std::min(read_size, read_samples);
        std::memcpy(&output[0], &mBuffer[currentRead],  read_size * sizeof(float));

        currentRead = (currentRead + read_size) & mMask;

        mReadIndex.store(currentRead, std::memory_order_release);

        read_frames = read_size / mChannels;
        return read_frames;
    }
    
    //* case 2:
    size_t read_size = mSize - currentRead;
    read_size = std::min(read_size, read_samples);
    std::memcpy(&output[0], &mBuffer[currentRead], read_size * sizeof(float));
    
    read_frames += read_size;
    currentRead = (currentRead + read_size) & mMask;

    if(read_size != read_samples){
        
        size_t new_size = std::min(
            (currentWrite - currentRead),
            (read_samples - read_size)
        );

        std::memcpy(&output[read_size], &mBuffer[currentRead], new_size * sizeof(float));

        currentRead = (currentRead + new_size) & mMask;
        read_frames += new_size;
    }

    mReadIndex.store(currentRead, std::memory_order_release);


    //* update the full flag only if it was full before read operation.
    /*
    * p = full flag is true before read operation.
    * q = update full flag to be false.
    * p <=> q
    */
    if(available_samples == mSize){
        mFullFlag.store(false, std::memory_order_release);
    }

    read_frames /= mChannels;
    return read_frames;
}
