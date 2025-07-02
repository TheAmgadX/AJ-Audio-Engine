#pragma once
#include<iostream>

namespace aj::io {
class AudioFile {
public:
    virtual void Read(const std::string &path) = 0;
    virtual void Write(const std::string &path) = 0;

    virtual ~AudioFile() = default;
};
};