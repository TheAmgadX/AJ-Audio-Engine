#include "file_io/file_utils.h"

#include <filesystem>
#include <algorithm>



bool AJ::utils::FileUtils::available_file_extension(std::string ext) {
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == "wav" || ext == "mp3";
}

bool AJ::utils::FileUtils::file_exists(std::string &path) {
    if(std::filesystem::exists(path) && std::filesystem::is_regular_file(path)){
        return true;
    }

    return false;
}

bool AJ::utils::FileUtils::trim_file_name(std::string &name) {
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

bool AJ::utils::FileUtils::valid_directory(std::string &directory) {
    if(std::filesystem::exists(directory)){
        return true;
    }

    return false;
}

bool AJ::utils::FileUtils::make_directory(std::string &directory){
    if(valid_directory(directory)){
        return true;
    }

    return std::filesystem::create_directory(directory);
}

std::string AJ::utils::FileUtils::generate_file_name(FileStreamingTypes type, std::string& extension){
    std::string name = "";
    switch (type)
    {
    case FileStreamingTypes::recording:
        name = "recording_session_";
        break;
    
    default:
        name = "session_";
        break;
    }

    auto now = std::chrono::system_clock::now();
    std::time_t date = std::chrono::system_clock::to_time_t(now);
    std::string date_s = std::ctime(&date);

    for(char &c : date_s){
        if(c == ' ' || c == ':'){
            c = '_';
        }
        if(c == '\n'){
            c = '.';
        }
    }
    int last_idx = date_s.size() - 1;
    if(date_s[last_idx] != '\n'){
        date_s[last_idx] = '.';
    }

    if(date_s[last_idx] != '.'){
        date_s += '.';
    }

    name += date_s;
    name += extension;

    return name;
}

std::string AJ::utils::FileUtils::get_file_extension(std::string& path) {
    size_t dotPos = path.find_last_of('.');
    
    if (dotPos == std::string::npos || dotPos == path.length() - 1) {
        return ""; // No extension or dot is the last character
    }

    return path.substr(dotPos + 1);
}

