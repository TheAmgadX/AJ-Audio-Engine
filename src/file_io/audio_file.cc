#include <filesystem>
#include <algorithm>
#include <set>
#include <sndfile.h> // docs: http://www.mega-nerd.com/libsndfile/api.html#open

#include "file_io/audio_file.h"
#include "core/types.h"

#include "file_io/audio_file.h"



bool AJ::io::AudioFile::write(){
    return false;
}

bool AJ::io::AudioFile::read(){
    return false;
}

bool AJ::io::AudioFile::_available_file_extension(std::string &ext) const {
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == ".wav" || ext == ".mp3";
}

AJ::FileExtension AJ::io::AudioFile::_stringToFileExtension(std::string &ext) {
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if(ext == ".wav") {
        return FileExtension::WAV;
    }
    
    if(ext == ".mp3"){
        return FileExtension::MP3;
    }

    return FileExtension::NotAvailable;
}

bool AJ::io::AudioFile::_validPath(String_c &path) const {
    if(std::filesystem::exists(path) && std::filesystem::is_regular_file(path)){
        return true;
    }

    return false;
}

bool AJ::io::AudioFile::_trimFileName(std::string &name) {
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

bool AJ::io::AudioFile::_validDirectory(String_c &path) const {
    if(std::filesystem::exists(path)){
        return true;
    }

    return false;
}

bool AJ::io::AudioFile::setWriteInfo(const AJ::AudioWriteInfo& info)
{
    if (info.bitdepth == BitDepth_t::Not_Supported) {
        std::cerr << "Invalid bit depth.\n";
        return false;
    }

    if (info.channels > kNumChannels || info.channels < 1) {
        std::cerr << "Invalid channels number.\n";
        return false;
    } 

    if (!_validDirectory(info.path)) {
        std::cerr << "Invalid path.\n";
        return false;
    }

    if (info.length <= 0 || info.length % info.channels != 0) {
        std::cerr << "Invalid file length.\n";
        return false;
    }

    if (info.format != ".wav" && info.format != ".mp3") {
        std::cerr << "Invalid file format.\n";
        return false;
    }

    const std::set<sample_c> validRates = {
        8000, 11025, 12000, 16000, 22050,
        24000, 32000, 44100, 48000
    };

    if (validRates.find(info.samplerate) == validRates.end()) {
        std::cerr << "Not supported sample rate.\n";
        return false;
    }

    // Assign validated info
    mWriteInfo = info;

    return true;
}


