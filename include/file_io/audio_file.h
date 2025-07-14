#pragma once
#include <iostream>

#include "core/types.h"


namespace AJ::io {

class AudioFile {
private:

    /// @brief check if the file extension available in the system from the path.
    /// @return return true if available false if not available.
    bool _available_file_extension(std::string &ext) const; 

    AJ::FileExtension _stringToFileExtension(std::string &ext);

    bool _validPath(String_c &path) const;

    bool _validDirectory(String_c &path) const;

    bool _trimFileName(std::string &name);

protected: 
    std::string mFileName;
    std::string mFilePath;
    AJ::FileExtension mExtension;
    AudioWriteInfo mWriteInfo;


public:
    virtual bool read();
    virtual bool write();

    ~AudioFile() = default;

    std::string FileName() const noexcept {
        return mFileName;
    }

    std::string FilePath() const noexcept {
        return mFilePath;
    }

    AJ::FileExtension FileExtension() const noexcept {
        return mExtension;
    }

    
    bool setFileName(std::string &name){
        if(_trimFileName(name)){
            mFileName = name;
            return true;
        }

        return false;
    }

    bool setFilePath(std::string &path){
        if(_validPath(path)){
            mFilePath = path;
            return true;
        }

        return false;
    }

    bool setFileExtension(std::string &ext){
        if(_available_file_extension(ext)){
            mExtension = _stringToFileExtension(ext);
            return true;
        }

        return false;
    }

    AudioFile(){
        pAudio = std::make_shared<AJ::AudioBuffer>();
    };

    bool setWriteInfo(const AJ::AudioWriteInfo& info);

    AudioSamples pAudio;
    AudioInfo mInfo;
};
};