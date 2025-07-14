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
    bool close_file(SNDFILE *file, bool return_val);
    BitDepth_t get_bit_depth(const SF_INFO &info);
    bool read_mono_data(SNDFILE *file);
    bool read_stereo_data(SNDFILE *file);

    void set_file_info(SF_INFO &info);
    bool write_samples_mono(SNDFILE *file);
    bool write_samples_stereo(SNDFILE *file);

public:    
    // File Methods
    bool read() override;
    bool write() override;
}; 
}