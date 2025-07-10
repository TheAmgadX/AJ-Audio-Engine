#pragma once

#include <vector>
#include <array>
#include <cstdint>
#include <memory>

#include "file_io/audio_file.h"
#include "constants.h"

namespace AJ {

/// @brief gain type
typedef float gain_t;

typedef float decay_t;

/// @brief sample type
typedef float sample_t;

/// @brief sample count
typedef int64_t sample_c;

/// @brief sample position 
typedef int64_t sample_pos;

using Float = std::vector<float>;
using Int8 = std::vector<int8_t>;
using Int16 = std::vector<int16_t>;
using Int32 = std::vector<int32_t>;

/// @brief A buffer structure representing audio samples organized by channel and processing block.
/// 
/// Structure:
/// - Outer array: one entry per channel (e.g. 1 for mono, 2 for stereo).
/// - Middle vector: one entry per processing blocks.
/// - Inner array: actual audio samples for a single block.
/// 
/// Use case: Block-based multichannel audio processing.
using AudioBufferBlocks = std::array<
        std::vector<
            std::array<float, kBlockSize>
        >,
kNumChannels>;

using AudioChannelBufferBlocks = std::vector<std::array<float, kBlockSize>>; 

/// @brief used in data object in the Audio File Class
using AudioSamples = std::shared_ptr<AudioBufferBlocks>;

/// @brief used in mAudioFiles in the engine class for storing all audio files
using Audio = std::shared_ptr<AJ::io::AudioFile>;


using String_c = const std::string; 

enum FileExtension {
    WAV = 1,
    MP3 = 2,
    NotAvailable = 3
};

/// @brief to represent WAV File Header used in WAV_File class
//? WAV File Encoding: http://soundfile.sapp.org/doc/WaveFormat/
//! default endians are little I mention if it's big endian.
struct WAVFileHeader{
    /* ============================================================= */
    /* =============          The RIFF Header          ============= */
    /* ============================================================= */
    char ChunkID[4]; //* contains RIFF in ASCII Form. big endian

    uint32_t ChunkSize; //* 4 bytes: Size of the file - 8 bytes for the ChunkID, ChunkSize fields

    char Format[4]; //* Contains the letters 'WAVE' big endian form
    
    /* ============================================================== */
    /* =============          The fmt Subchunk          ============= */
    /* ============================================================== */

    // the fmt subchunk:
    char SubChunk1ID[4]; //* Contains 'fmt' big endian form

    uint32_t SubChunk1Size; /* 16 for PCM, This is the size of the
    rest of the Subchunk which follows this number */

    uint16_t AudioFormat; //* 2 bytes. 1 for PCM values other than 1 indicates some compression

    uint16_t NumChannels; //* mono = 1, stereo = 2, ...etc

    uint32_t SampleRate; //* sample rate 44100, 8000, ...etc

    uint32_t ByteRate; 

    uint16_t BlockAlign;

    uint16_t BitsPerSample; 


    /* =============================================================== */
    /* =============          The Data Subchunk          ============= */
    /* =============================================================== */

    char SubChunk2ID[4]; //* Contains 'data' big endian form.
    uint32_t SubChunk2Size; //* This is the number of bytes in the data.

}__attribute__((packed)); //! to avoid compiler padding
/*
    - compiler add padding bytes inside the struct to align fields for faster memory access
        ==> but we are reading files so it will corrupt the file header reading.
            since it must match exactly the file binary structure or it will not work.
*/

struct AudioInfo {
    sample_c length;
    sample_c samplerate;
    uint8_t channels;
    uint8_t bitdepth;
    std::string format;
    bool seekable;
};

};

