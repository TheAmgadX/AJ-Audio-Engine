#pragma once
#include <iostream>
#include <string>

#include "core/types.h"

namespace AJ::io {

class AudioFile {
private:

    /// @brief check if the file extension available in the system from the path.
    /// @return return true if available false if not available.
    bool _available_file_extension(String_c path) const; 

    FileExtension _stringToFileExtension(String ext);

    bool _validPath(String_c path) const;

    bool _trimFileName(String name);

protected: 
    std::string mFileName;
    const char *mFilePath;
    FileExtension mExtension;

public:
    virtual bool read();
    virtual bool write();

    ~AudioFile() = default;
    AudioFile() = default;

    AudioFile(const char *path, String_c name, FileExtension extension) : mFileName(name),
                        mFilePath(path), mExtension(extension) {}

    AudioFile(const char *path, String_c name) : mFileName(name), mFilePath(path){}


    std::string FileName() const noexcept {
        return mFileName;
    }

    bool setFileName(String name){
        if(_trimFileName(name)){
            mFileName = name;
            return true;
        }

        return false;
    }

    const char * FilePath() const noexcept {
        return mFilePath;
    }

    bool setFilePath(const char *path){
        if(_validPath(path)){
            mFilePath = path;
            return true;
        }

        return false;
    }

    FileExtension FileExtension() const noexcept {
        return mExtension;
    }

    bool setFileExtension(String ext){
        if(_available_file_extension(ext)){
            mExtension = _stringToFileExtension(ext);
            return true;
        }

        return false;
    }

    AudioSamples pAudio;
    AudioInfo mInfo;
};
};