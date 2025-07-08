#pragma once
#include <iostream>
#include <vector>
#include <cstdint> // for some data types like uint32_t
#include <fstream>
#include <string>

#include "audio_file.h"
#include "core/types.h"

namespace AJ::io {




class WAV_File : public io::AudioFile {
private:
    /*
        Reading must be in the int(x)_t where x = bit depth.
        but for more accuracy while calculation and DSP filters
        we need to make it float and before saving we need to make it int(x)_t again.

        as example:
            int16_t [-32768, 32767] to float[-1.0f, 0.9999f]
    */
    static Int16 convert_float_to_int16(const Float& float_samples);

    static Int8 convert_float_to_int8(const Float& float_samples);

    static Int32 convert_float_to_int32(const Float& float_samples);

    static Float convert_to_float(const void *data, const uint8_t &bitsPerSample,
        const uint &sample_count);

    /* ====================================================== */ 
    void read_WAV_file_header(std::ifstream &file);

    /// used in writing bytes into the files in exporting phase.
    /// @param file => output file.
    /// @param val => value to write [char or int].
    /// @param byte_size => number of bytes to write.
    /// @return bool if the write done successfully or not.
    bool write_as_bytes(std::ofstream &file, const void *val, const size_t byte_size);

protected:

    virtual bool _available_file_extension(const std::string &path) const;

public:

    // File Attributes
    WAVFileHeader mHeader;
    std::vector<float> mAudioData;


    // File Methods
    void read() override;
    void write() override;
}; 
};