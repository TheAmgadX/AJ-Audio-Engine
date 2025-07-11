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
    BitDepth_t get_bit_depth(const SF_INFO &info);
    bool read_mono_data(SNDFILE *file);
    bool read_stereo_data(SNDFILE *file);
public:    

    // File Methods
    bool read() override;
    bool write() override;
}; 
};