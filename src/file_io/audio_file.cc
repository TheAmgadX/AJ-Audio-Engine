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

bool AJ::io::AudioFile::setWriteInfo(
    const sample_c &length,
    const sample_c &samplerate,
    const uint8_t &channels,
    const BitDepth_t &bitdepth,
    const AJ::FileExtension &format,
    const bool &seekable,
    const std::string &path,
    const std::string &name)
{
     if(bitdepth == BitDepth_t::Not_Supported){
        std::cerr << "Invlaid bit depth.\n";
        return false;
    }

    if(channels > kNumChannels || channels < 1){
        std::cerr << "Invlid channels number.\n";
        return false;
    }
    
    if(!_validPath(path)){
        std::cerr << "Invlid path.\n";
        return false;
    }

    if(length <= 0 || length % channels != 0){
        std::cerr << "Invlid file length.\n";
        return false;
    }

    if(format == FileExtension::WAV){
        mWriteInfo.format = ".wav";
    } else if (format == FileExtension::MP3){
        mWriteInfo.format = ".mp3";
    } else {
        std::cerr << "Invalid file format.\n";
        return false;
    }

    const std::set<sample_c> validRates = {
        8000,11025,12000,16000,22050,
        24000,32000,44100,48000
    };
    
    if (validRates.find(samplerate) == validRates.end()) {
        std::cerr << "Not supported sample rate\n";
        return false;
    }
   

    mWriteInfo.bitdepth = bitdepth;
    mWriteInfo.channels = channels;
    mWriteInfo.length = length;
    mWriteInfo.name = name;
    mWriteInfo.path = path;
    mWriteInfo.samplerate = samplerate;
    mWriteInfo.seekable = seekable;

    return true;
}

