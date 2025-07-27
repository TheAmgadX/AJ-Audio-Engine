#pragma once
#include "audio_file.h"

namespace AJ::io {
class MP3_File final : public AJ::io::AudioFile{
public:
    bool read(AJ::error::IErrorHandler &handler) override;
    bool write(AJ::error::IErrorHandler &handler) override;
};
};