#pragma once
#include <iostream>
#include <vector>
#include <cstdint>
#include <fstream>
#include <string>
#include <sndfile.h> // docs: http://www.mega-nerd.com/libsndfile/api.html#open


#include "audio_file.h"
#include "core/types.h"


namespace AJ::io {


class WAV_File final: public io::AudioFile {
private:
    bool close_file(SNDFILE *file, bool return_val, AJ::error::IErrorHandler &handler);
    BitDepth_t get_bit_depth(const SF_INFO &info);
    bool read_mono_data(SNDFILE *file, AJ::error::IErrorHandler &handler);
    bool read_stereo_data(SNDFILE *file, AJ::error::IErrorHandler &handler);

    void set_file_info(SF_INFO &info);
    bool write_samples_mono(SNDFILE *file, AJ::error::IErrorHandler &handler);
    bool write_samples_stereo(SNDFILE *file, AJ::error::IErrorHandler &handler);

public:    
    // File Methods
    bool read(AJ::error::IErrorHandler &handler) override;
    bool write(AJ::error::IErrorHandler &handler) override;
}; 
}