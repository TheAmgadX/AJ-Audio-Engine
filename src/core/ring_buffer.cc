#include "core/ring_buffer.h"

bool AJ::utils::RingBuffer::writeFrame(const float *samples) noexcept {
    if(!samples){
        return false;
    }
    
    size_t currentWrite = mWriteIndex.load(std::memory_order_relaxed);
    
    //* check if available space:
    size_t space = getWriteSpace();

    if(space < mChannels) return false;

    for(int ch = 0; ch < mChannels; ++ch){
        mBuffer[currentWrite] = samples[ch];
        currentWrite = (currentWrite + 1) & mSizeMask;
    }

    mWriteIndex.store(currentWrite, std::memory_order_release);
    return true;
}

size_t AJ::utils::RingBuffer::writeFrames(const float* input, const size_t frame_count) noexcept {
    if(!input) return 0;

    size_t write_samples = getWriteSpace();
    size_t write_frames = write_samples / mChannels;

    size_t samples_count = frame_count * mChannels;
    size_t written_frames;
    
    if(write_samples < samples_count){
        written_frames = write_frames;
        samples_count = write_samples;
    } else {
        written_frames = frame_count;
    }

    size_t currentWrite = mWriteIndex.load(std::memory_order_relaxed);

    size_t first = std::min(mSize - currentWrite, samples_count);

    std::memcpy(&mBuffer[currentWrite], &input[0], first * sizeof(float));
    currentWrite = (currentWrite + first) & mSizeMask;
    
    //* write the rest of the samples if exist
    if(first != samples_count){
        size_t second = samples_count - first;
        std::memcpy(&mBuffer[currentWrite], &input[first], second * sizeof(float));
        currentWrite = (currentWrite + second) & mSizeMask;
    }

    mWriteIndex.store(currentWrite, std::memory_order_release);

    return written_frames;
}

bool AJ::utils::RingBuffer::readFrame(float *output) noexcept {
    if(output == nullptr) return false;

    size_t currentWrite = mWriteIndex.load(std::memory_order_acquire);
    size_t available_samples = getReadAvailable(currentWrite);
    
    if(available_samples == 0 || !output){
        return false;
    }

    size_t currentRead = mReadIndex.load(std::memory_order_relaxed);

    for(uint8_t ch = 0; ch < mChannels; ++ch){
        output[ch] = mBuffer[currentRead];
        currentRead = (currentRead + 1) & mSizeMask;
    }

    mReadIndex.store(currentRead, std::memory_order_release);
    return true;
}

size_t AJ::utils::RingBuffer::readFrames(float *output, const size_t frames_count) noexcept {
    if(output == nullptr || frames_count == 0) return 0;

    size_t currentWrite = mWriteIndex.load(std::memory_order_acquire);

    size_t available_samples = getReadAvailable(currentWrite);
    
    if(available_samples == 0 || !output) return 0;

    size_t read_frames = 0;

    size_t samples_count = frames_count * mChannels;

    if(available_samples < samples_count){
        samples_count = available_samples;
    }

    //* read data from mBuffer to output.
    /*
        * we have two cases:
            - case 1: read index < write index:
                read from read index to min(write index - 1, samples_count).
            
            -case 2: read index > write index:
                read from read index to first = min(end, samples_count).
                if there is remaining samples (first != samples_count): 
                    read from the beg to second = min(write index - 1, samples_count - first).
    */
    size_t currentRead = mReadIndex.load(std::memory_order_relaxed);

    if(currentRead < currentWrite){
        size_t read_size = currentWrite - currentRead;
        read_size = std::min(read_size, samples_count);
        std::memcpy(&output[0], &mBuffer[currentRead],  read_size * sizeof(float));

        currentRead = (currentRead + read_size) & mSizeMask;

        mReadIndex.store(currentRead, std::memory_order_release);

        read_frames = read_size / mChannels;
        return read_frames;
    }
    
    size_t read_size = mSize - currentRead;
    read_size = std::min(read_size, samples_count);
    std::memcpy(&output[0], &mBuffer[currentRead], read_size * sizeof(float));
    
    read_frames += read_size;
    currentRead = (currentRead + read_size) & mSizeMask;

    if(read_size != samples_count){
        
        size_t new_size = std::min(
            (currentWrite - currentRead),
            (samples_count - read_size)
        );

        std::memcpy(&output[read_size], &mBuffer[currentRead], new_size * sizeof(float));

        currentRead = (currentRead + new_size) & mSizeMask;
        read_frames += new_size;
    }

    mReadIndex.store(currentRead, std::memory_order_release);

    read_frames /= mChannels;
    return read_frames;
}
