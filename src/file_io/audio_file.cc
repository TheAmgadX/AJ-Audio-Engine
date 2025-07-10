#pragma once

#include <filesystem>
#include <string>
#include <algorithm>
#include <sndfile.h> // docs: http://www.mega-nerd.com/libsndfile/api.html#open

#include "include/file_io/audio_file.h"
#include "include/core/types.h"

bool AJ::io::AudioFile::_available_file_extension(String_c &ext) const {
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