#include <cstring>
#include <utility>
#include <vector>
#include <algorithm>

#include "core/types.h"
#include "core/errors.h"
#include "core/error_handler.h"

#include "editing/insert.h"

bool AJ::editing::insert::Insert::insert(std::shared_ptr<AJ::io::AudioFile> file, AudioSamples audio){
    auto main_buffer = file->pAudio;

    sample_c buffer_size = audio->at(0).size();
    sample_c main_buffer_size = main_buffer->at(0).size();
    sample_c new_size = buffer_size + main_buffer_size;


    for(int ch = 0; ch < file->mInfo.channels; ++ch){
        Float new_buffer(new_size);
        
        new_buffer.insert (
            new_buffer.begin(), // destination pos.
            main_buffer->at(ch).begin(), // source start pos.
            main_buffer->at(ch).begin() + mInsertAt // source end pos.
        );
        

        std::move(
            audio->at(ch).begin(), 
            audio->at(ch).end(), 
            new_buffer.begin() + mInsertAt
        );

        std::move(
            main_buffer->at(ch).begin() + mInsertAt,
            main_buffer->at(ch).end(),
            new_buffer.begin() + mInsertAt + buffer_size
        );

        // file->pAudio->at(ch).clear();
        file->pAudio->at(ch).resize(new_size);

        file->pAudio->at(ch) = std::move(new_buffer);
    }

    file->mInfo.length += buffer_size * file->mInfo.channels;

    return true;
}

bool AJ::editing::insert::Insert::pushBack(std::shared_ptr<AJ::io::AudioFile> file, AudioSamples audio){

    auto main_buffer = file->pAudio;

    sample_c buffer_size = audio->at(0).size();
    sample_c main_buffer_size = main_buffer->at(0).size();
    sample_c new_size = buffer_size + main_buffer_size;

    for(int ch = 0; ch < file->mInfo.channels; ++ch){
        file->pAudio->at(ch).resize(new_size);

        std::move(
            audio->at(ch).begin(), 
            audio->at(ch).end(), 
            main_buffer->at(ch).begin() + main_buffer_size
        );
    }

    file->mInfo.length += buffer_size * file->mInfo.channels;

    return true;
}

bool AJ::editing::insert::Insert::pushFront(std::shared_ptr<AJ::io::AudioFile> file, AudioSamples audio){
        auto main_buffer = file->pAudio;

    sample_c buffer_size = audio->at(0).size();
    sample_c main_buffer_size = main_buffer->at(0).size();
    sample_c new_size = buffer_size + main_buffer_size;


    for(int ch = 0; ch < file->mInfo.channels; ++ch){
        Float new_buffer(new_size);

        std::move(
            audio->at(ch).begin(), 
            audio->at(ch).end(), 
            new_buffer.begin()
        );

        std::move(
            main_buffer->at(ch).begin(),
            main_buffer->at(ch).end(),
            new_buffer.begin() + buffer_size
        );

        file->pAudio->at(ch).resize(new_size);

        file->pAudio->at(ch) = std::move(new_buffer);
    }

    file->mInfo.length += buffer_size * file->mInfo.channels;

    return true;
}

bool AJ::editing::insert::Insert::process(std::shared_ptr<AJ::io::AudioFile> file,
    AudioSamples audio, AJ::error::IErrorHandler& handler){

    if(audio->size() == 0){
        const std::string message = "Invalid audio buffers.";
        handler.onError(error::Error::InvalidAudioLength, message);
        return false;    
    }

    if(audio->at(0).size() == 0){
        const std::string message = "Invalid audio buffers, insert buffer is empty.";
        handler.onError(error::Error::InvalidAudioLength, message);
        return false; 
    }

    if(file->mInfo.channels == 2 && audio->at(1).size() == 0){
        const std::string message = "Invalid audio buffers, expect stereo buffer to insert.";
        handler.onError(error::Error::InvalidAudioLength, message);
        return false; 
    }

    if(mInsertAt == -1){
        const std::string message = "Insert at index is not initialized. Please use setInsertAt method before calling process().\n";
        handler.onError(error::Error::InvalidProcessingRange, message);
        return false;
    }

    if(mInsertAt > file->pAudio->at(0).size()){
        const std::string message = "invalid insert at index, it exceeds the buffer size.\n";
        handler.onError(error::Error::InvalidProcessingRange, message);
        return false;
    } 

    if(mInsertAt == 0)
        return pushFront(file, audio);
    
    else if(mInsertAt == file->pAudio->at(0).size())
        return pushBack(file, audio);
    
    else 
        return insert(file, audio);
}