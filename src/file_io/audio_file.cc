#include <filesystem>
#include <algorithm>
#include <set>
#include <sndfile.h> // docs: http://www.mega-nerd.com/libsndfile/api.html#open

#include "file_io/audio_file.h"
#include "core/types.h"

#include "file_io/audio_file.h"
#include "core/error_handler.h"


bool AJ::io::AudioFile::available_file_extension(std::string ext) const {
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == "wav" || ext == "mp3";
}

bool AJ::io::AudioFile::validPath(String_c &path) const {
    if(std::filesystem::exists(path) && std::filesystem::is_regular_file(path)){
        return true;
    }

    return false;
}

bool AJ::io::AudioFile::trimFileName(std::string &name) {
    if(name == "") 
        return false;
    
    auto start = std::find_if_not(name.begin(), name.end(), ::isspace);
    auto end = std::find_if_not(name.rbegin(), name.rend(), ::isspace).base();

    // if no non white spaces
    if(start >= end)
        return false;


    name =  std::string(start, end);

    return true; 
}

bool AJ::io::AudioFile::validDirectory(String_c &path) const {
    if(std::filesystem::exists(path)){
        return true;
    }

    return false;
}

std::string AJ::io::AudioFile::getFileExtension(const std::string& path) {
    size_t dotPos = path.find_last_of('.');
    
    if (dotPos == std::string::npos || dotPos == path.length() - 1) {
        return ""; // No extension or dot is the last character
    }

    return path.substr(dotPos + 1);
}

bool AJ::io::AudioFile::setWriteInfo(const AJ::AudioWriteInfo& info, AJ::error::IErrorHandler &handler)
{
    // bit depth is important only for wav files reading / writing.
    if(mInfo.format == ".wav" && info.bitdepth == BitDepth_t::Not_Supported){
        const std::string message = "Unsupported audio bit depth. Please use a supported bit depth format.\n";

        handler.onError(AJ::error::Error::UnsupportedFileFormat, message);
        return false;
    } 

    if (info.channels > kNumChannels || info.channels < 1) {
        const std::string message = "Error: Unsupported channels number only support mono and stereo.\n";

        handler.onError(AJ::error::Error::UnsupportedFileFormat, message);
        return false;
    } 

    if (!validDirectory(info.path)) {
        const std::string message = "Error: invalid path.\n";

        handler.onError(AJ::error::Error::InvalidFilePath, message);
        return false;
    }

    if (info.length <= 0 || info.length % info.channels != 0) {
        const std::string message = "Error: invalid file length.\n";

        handler.onError(AJ::error::Error::InvalidAudioLength, message);
        return false;
    }

    if (info.format != ".wav" && info.format != ".mp3") {
        const std::string message = "Error: invalid file format only support mp3 and wav.\n";

        handler.onError(AJ::error::Error::UnsupportedFileFormat, message);
        return false;
    }

    const std::set<sample_c> validRates = {
        8000, 11025, 12000, 16000, 22050,
        24000, 32000, 44100, 48000
    };

    if (validRates.find(info.samplerate) == validRates.end()) {
        const std::string message = "Error: unsupported samplerate.\n";

        handler.onError(AJ::error::Error::InvalidSampleRate, message);
        return false;
    }

    // Assign validated info
    mWriteInfo = info;

    return true;
}


