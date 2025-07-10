#include <iostream>
#include <vector>
#include <cstdint> // for some data types like uint32_t
#include <fstream>

#include "include/file_io/wav_file.h"
#include <sndfile.h> // docs: http://www.mega-nerd.com/libsndfile/api.html#open

// TODO: Continue this function and write functions tomorrow.
bool AJ::io::WAV_File::read() {
    SF_INFO *sf_info;

    SNDFILE *snd_file = sf_open(mFilePath, SFM_READ, sf_info);

}   