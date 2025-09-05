#include "file_io/file_streamer.h"
#include <sndfile.h>

#include <unordered_set>
#include <chrono>
#include <thread>

void AJ::io::file_streamer::FileStreamer::set_file_info(SF_INFO& info){
    info.channels = pWriteInfo->channels;
    info.samplerate = pWriteInfo->samplerate;
    info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
}

bool AJ::io::file_streamer::FileStreamer::close_file(SNDFILE *file, bool return_val, AJ::error::IErrorHandler &handler){
    if (sf_close(file) != 0){
        const std::string message = "Failed to close audio file. Resource may still be in use.\n";

        handler.onError(AJ::error::Error::FileClosingError, message);
        return false;
    } // sf_close returns 0 in success

    return return_val;
}

bool AJ::io::file_streamer::FileStreamer::read(AJ::error::IErrorHandler &handler){
    //TODO: implement read in chunks function.
    return false;
}

void AJ::io::file_streamer::FileStreamer::writeInterleaved(SNDFILE* file,
    AJ::utils::Buffer* buffer, AJ::error::IErrorHandler& handler){
    sf_count_t written = sf_writef_float(file, buffer->data, buffer->frames);

    if (written != static_cast<sf_count_t>(buffer->frames)) {
        const std::string message = "Error: failed to write audio samples to file " 
            + pWriteInfo->path + "/" + pWriteInfo->name + "\n";
            
        handler.onError(AJ::error::Error::FileWriteError, message);
    }
}

bool AJ::io::file_streamer::FileStreamer::write(AJ::error::IErrorHandler &handler){
    /*
        * 1- check whether queue is valid or not.
        * 2- create a file and open it to be ready for the writing.
        * 3- listen to the queue -> pop from queue -> write to disk -> push into buffer pool 
     */
    if(!pQueue->isValid()){
        return false;
    }

    SF_INFO info;

    set_file_info(info);
    
    std::string fullPath = pWriteInfo->path + "/" + pWriteInfo->name;

    SNDFILE *file = sf_open(fullPath.c_str(), SFM_WRITE, &info);

    if(!file){
        const std::string message = "Error: Couldn't create file at: " + fullPath + "\n";

        handler.onError(AJ::error::Error::FileOpenError, message);
        return false;
    }

    AJ::utils::Buffer* buffer = nullptr;

    while(!pStopFlag->flag.load(std::memory_order_acquire)){
        buffer = pQueue->pop();

        if(!buffer){
            std::this_thread::sleep_for(std::chrono::microseconds(10));
            continue;
        } 

        writeInterleaved(file, buffer, handler);

        pBufferPool->push(buffer, handler);
    }


    // consume the rest of the buffers if exists.
    while(true){
        buffer = pQueue->pop();

        if(!buffer){
            close_file(file, true, handler);
            return true;
        }

        writeInterleaved(file, buffer, handler);

        pBufferPool->push(buffer, handler);
    }

    // this point will never be reached but to stop compiler warnings.
    return true;
}

bool AJ::io::file_streamer::FileStreamer::setReadInfo(const AudioInfo& info, AJ::error::IErrorHandler &handler){
    // TODO: implement this function.
    return false;
}

bool AJ::io::file_streamer::FileStreamer::setWriteInfo(const AudioWriteInfo& info, AJ::error::IErrorHandler &handler){
    if (info.channels > kNumChannels || info.channels < 1) {
        const std::string message = "Error: Unsupported channels number only support mono and stereo.\n";

        handler.onError(AJ::error::Error::InvalidChannelCount, message);
        return false;
    } 

    const std::unordered_set<sample_c> validRates = {
        8000, 11025, 12000, 16000, 22050,
        24000, 32000, 44100, 48000
    };

    if (validRates.find(info.samplerate) == validRates.end()) {
        const std::string message = "Error: unsupported samplerate.\n";

        handler.onError(AJ::error::Error::InvalidSampleRate, message);
        return false;
    }

    if(!pWriteInfo){
        pWriteInfo = std::make_shared<AJ::AudioWriteInfo>();
    }

    pWriteInfo->channels = info.channels;
    pWriteInfo->samplerate = info.samplerate;

    pWriteInfo->path = mStreamingInfo.directory;
    pWriteInfo->name = mStreamingInfo.name;

    return true;
}