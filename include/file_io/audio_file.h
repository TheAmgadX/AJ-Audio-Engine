#pragma once
#include <iostream>
#include <string>

#include "core/types.h"

namespace AJ::io {

class AudioFile {
private:

    /// @brief check if the file extension available in the system from the path.
    /// @return return true if available false if not available.
    bool _available_file_extension(String_c &path) const; 

    FileExtension _stringToFileExtension(std::string &ext);

    bool _validPath(String_c &path) const;

    bool _trimFileName(std::string &name);

protected: 
    std::string mFileName;
    std::string mFilePath;
    FileExtension mExtension;

public:
    virtual bool read();
    virtual bool write();

    AudioFile(){
        pAudio = std::make_shared<AudioBufferBlocks>();
    };

    ~AudioFile() = default;

    std::string FileName() const noexcept {
        return mFileName;
    }

    bool setFileName(std::string &name){
        if(_trimFileName(name)){
            mFileName = name;
            return true;
        }

        return false;
    }

    std::string FilePath() const noexcept {
        return mFilePath;
    }

    bool setFilePath(std::string &path){
        if(_validPath(path)){
            mFilePath = path;
            return true;
        }

        return false;
    }

    FileExtension FileExtension() const noexcept {
        return mExtension;
    }

    bool setFileExtension(std::string &ext){
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