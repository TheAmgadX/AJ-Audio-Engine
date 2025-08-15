#include <cstring>

#include "core/types.h"
#include "core/errors.h"
#include "core/error_handler.h"

#include "editing/cut.h"

bool AJ::editing::cut::Cut::process(std::shared_ptr<AJ::io::AudioFile> file,
    AJ::error::IErrorHandler& handler){
    if(mStart == -1){
        const std::string message = "Cut range not initialized. Please use setRange method before calling process().\n";
        handler.onError(error::Error::InvalidProcessingRange, message);
        return false;
    }
    
    int size = file->pAudio->at(0).size();

    if(mStart > mEnd || mStart >= size){
        const std::string message = 
            "Invalid cut range. Expected 0 <= start <= end < buffer.size(). "
            "Received start = " + std::to_string(mStart) +
            ", end = " + std::to_string(mEnd) +
            ", buffer.size() = " + std::to_string(size) + ".";

        
        handler.onError(error::Error::InvalidProcessingRange, message);
        return false;
    }

    // If the whole buffer is being removed
    if(mStart == 0 && mEnd == size - 1){
        for(int i = 0; i < file->mInfo.channels; ++i){
            file->pAudio->at(i).clear();
        }
        file->mInfo.length = 0;
        return true;
    }

    size_t count_to_remove = mEnd - mStart + 1; // size of the removed part.
    size_t tail = size - (mEnd + 1); // elements after mEnd.
    for(int i = 0; i < file->mInfo.channels; ++i){
        memmove(
            file->pAudio->at(i).data() + mStart, // destination
            file->pAudio->at(i).data() + mEnd + 1, // source
            tail * sizeof(float) // number of bytes to move.
        );
        file->pAudio->at(i).resize(size - count_to_remove);
    }
    
    file->mInfo.length = (size - count_to_remove) * file->mInfo.channels;

    return true;
}