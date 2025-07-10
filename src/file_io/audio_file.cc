#pragma once

#include <filesystem>
#include <string>
#include <algorithm>

#include "include/file_io/audio_file.h"
#include "include/core/types.h"

// TODO: make function to read the file and know its type if not included in the path
bool AJ::io::AudioFile::_available_file_extension(String_c path) const {
    String_c ext = std::filesystem::path(path).extension().string();

    if(ext == ""){
        std::cerr << "file extension is not included in the path.\n";
        return false;
    }

    // to lower case convertion
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    return ext == ".wav" || ext == ".mp3";
}

AJ::FileExtension AJ::io::AudioFile::_stringToFileExtension(String ext) {
    
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if(ext == ".wav") {
        return FileExtension::WAV;
    }
    
    if(ext == ".mp3"){
        return FileExtension::MP3;
    }

    return FileExtension::NotAvailable;
}

bool AJ::io::AudioFile::_validPath(String_c path) const {
    if(std::filesystem::exists(path) && std::filesystem::is_regular_file(path)){
        return true;
    }

    return false;
}

bool AJ::io::AudioFile::_trimFileName(String name) {
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