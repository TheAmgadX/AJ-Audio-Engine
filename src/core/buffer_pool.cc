#include "core/buffer_pool.h"

size_t AJ::utils::Queue::currentSize() {
    size_t currentRead  = mReadIndex.load(std::memory_order_acquire);
    size_t currentWrite = mWriteIndex.load(std::memory_order_acquire);

    return getAvailableBuffers(currentRead, currentWrite);
}

bool AJ::utils::Queue::push(Buffer *buffer) noexcept {
    if(!buffer){
        return false;
    }

    size_t currentWrite = mWriteIndex.load(std::memory_order_relaxed); 

    size_t space = getFreeSpace(mWriteIndex);

    if(space == 0){
        return false;
    }
    
    mQueue[currentWrite] = buffer;
    
    mWriteIndex.store((currentWrite + 1) & mMask, std::memory_order_release);
    
    if(space == 1){
        mFullFlag.store(true, std::memory_order_release);
    }

    return true;
}

AJ::utils::Buffer* AJ::utils::Queue::pop() noexcept {
    size_t currentWrite = mWriteIndex.load(std::memory_order_acquire);
    
    size_t currentRead = mReadIndex.load(std::memory_order_relaxed);
    
    size_t buffers = getAvailableBuffers(currentWrite, currentRead);

    if(buffers == 0){
        return nullptr;
    }
    
    Buffer* buffer = mQueue[currentRead];
    
    size_t new_index = (currentRead + 1) & mMask;
    
    mReadIndex.store(new_index, std::memory_order_release);
    
    if(buffers == mQueueSize){
        mFullFlag.store(false, std::memory_order_release);
    }

    return buffer;
}