#pragma once
#include <iostream>
#include <string>

#include "core/types.h"

namespace AJ::io {

class AudioFile {
private:
    virtual bool _available_file_extension(const std::string &path) const = 0; 
public:
    virtual void read() = 0;
    virtual void write() = 0;

    virtual ~AudioFile() = default;

    AudioFile(){}

    AudioFile(String_c path, String_c name, FileExtension extension) : mFileName(name),
                        mFilePath(path), mExtension(extension) {}

    AudioFile(String_c path, String_c name) : mFileName(name), mFilePath(path){}

    std::string mFileName;
    std::string mFilePath;
    FileExtension mExtension;

    Float mRightChan;
    Float mLeftChan;
};
};