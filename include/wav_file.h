#pragma once
#include <iostream>
#include <vector>
#include <cstdint> // for some data types like uint32_t
#include <fstream>
#include <string>

#include "audio_file.h"


namespace aj::io {

class WAV_File : public io::AudioFile {
private:
    /*
        Reading must be in the int(x)_t where x = bit depth.
        but for more accuracy while calculation and DSP filters
        we need to make it float and before saving we need to make it int(x)_t again.

        as example:
            int16_t [-32768, 32767] to float[-1.0f, 0.9999f]
    */
    static std::vector<int16_t> convert_float_to_int16(const std::vector<float>& float_samples);

    static std::vector<int8_t> convert_float_to_int8(const std::vector<float>& float_samples);

    static std::vector<int32_t> convert_float_to_int32(const std::vector<float>& float_samples);

    static std::vector<float> convert_to_float(const void *data, uint8_t bitsPerSample,int sample_count);

    /* ====================================================== */ 
    void read_WAV_File_Header(std::ifstream &file, std::string fpath);

    bool write_as_bytes(std::ofstream &file, const void *val, const size_t byte_size);
    
public:
    // WAV File Encoding: http://soundfile.sapp.org/doc/WaveFormat/
    // default endians are little I mention if it's big endian.
    struct FileHeader{
        /* ============================================================= */
        /* =============          The RIFF Header          ============= */
        /* ============================================================= */
        char ChunkID[4]; // contains RIFF in ASCII Form. big endian

        uint32_t ChunkSize; // 4 bytes: Size of the file - 8 bytes for the ChunkID, ChunkSize fields

        char Format[4]; // Contains the letters 'WAVE' big endian form
        
        /* ============================================================== */
        /* =============          The fmt Subchunk          ============= */
        /* ============================================================== */

        // the fmt subchunk:
        char SubChunk1ID[4]; // Contains 'fmt' big endian form

        uint32_t SubChunk1Size; /* 16 for PCM, This is the size of the
        rest of the Subchunk which follows this number */

        uint16_t AudioFormat; // 2 bytes. 1 for PCM values other than 1 indicates some compression

        uint16_t NumChannels; // mono = 1, stereo = 2, ...etc

        uint32_t SampleRate; // sample rate 44100, 8000, ...etc

        uint32_t ByteRate; 

        uint16_t BlockAlign;

        uint16_t BitsPerSample; 


        /* =============================================================== */
        /* =============          The Data Subchunk          ============= */
        /* =============================================================== */

        char SubChunk2ID[4]; // Contains 'data' big endian form.
        uint32_t SubChunk2Size; // This is the number of bytes in the data.

    }__attribute__((packed)); // to avoid compiler padding
    /*
        - compiler add padding bytes inside the struct to align fields for faster memory access
            ==> but we are reading files so it will corrupt the file header reading.
                since it must match exactly the file binary structure or it will not work.
    */


    // File Attributes
    FileHeader header;
    std::vector<float> audio_data;


    // File Methods
    void read(const std::string &path) override;
    void write(const std::string &path) override;
}; 
};