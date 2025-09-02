#include "file_io/file_streamer.h"
#include "core/globals.h"

#include <unordered_set>

bool AJ::io::file_streamer::FileStreamer::read(AJ::error::IErrorHandler &handler){
    //TODO: implement read in chunks function.
    return false;
}

bool AJ::io::file_streamer::FileStreamer::write(AJ::error::IErrorHandler &handler){
    
}


bool AJ::io::file_streamer::FileStreamer::setReadInfo(const AudioInfo& info, AJ::error::IErrorHandler &handler){

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

    pWriteInfo->channels = info.channels;
    pWriteInfo->samplerate = info.samplerate;

    pWriteInfo->path = mStreamingInfo.directory;

    return true;
}