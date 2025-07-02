#pragma once
#include<iostream>

namespace aj::io {
class AudioFile {
public:
    virtual void read(const std::string &path) = 0;
    virtual void write(const std::string &path) = 0;

    virtual ~AudioFile() = default;
};
};