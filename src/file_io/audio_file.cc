#include <filesystem>
#include <algorithm>
#include <unordered_set>
#include <sndfile.h> // docs: http://www.mega-nerd.com/libsndfile/api.html#open

#include "file_io/audio_file.h"
#include "core/types.h"

#include "file_io/audio_file.h"
#include "core/error_handler.h"

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
    
    std::string path = info.path;
    if (!AJ::utils::FileUtils::valid_directory(path)) {
        const std::string message = "Error: invalid path.\n";

        handler.onError(AJ::error::Error::InvalidFilePath, message);
        return false;
    }

    if (info.length < 0 || info.length % info.channels != 0) {
        const std::string message = "Error: invalid file length.\n";

        handler.onError(AJ::error::Error::InvalidAudioLength, message);
        return false;
    }

    if (info.format != ".wav" && info.format != ".mp3") {
        const std::string message = "Error: invalid file format only support mp3 and wav.\n";

        handler.onError(AJ::error::Error::UnsupportedFileFormat, message);
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

    // Assign validated info
    mWriteInfo = info;

    return true;
}


