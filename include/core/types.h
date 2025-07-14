#pragma once

#include <vector>
#include <array>
#include <cstdint>
#include <memory>

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
using AudioBuffer = std::array<Float, kNumChannels>;

/// @brief used in data object in the Audio File Class
using AudioSamples = std::shared_ptr<AudioBuffer>;


using String_c = const std::string; 

enum FileExtension {
    WAV = 1,
    MP3 = 2,
    NotAvailable = 3
};

enum BitDepth_t{
    int_8,
    int_16,
    int_24,
    int_32,
    float_32,
    float_64,
    Not_Supported
};

struct AudioInfo {
    sample_c length;
    sample_c samplerate;
    uint8_t channels;
    BitDepth_t bitdepth;
    std::string format;
    bool seekable;
};

struct AudioWriteInfo {
    sample_c length;
    sample_c samplerate;
    uint8_t channels;
    BitDepth_t bitdepth;
    std::string format;
    bool seekable;

    std::string path;
    std::string name;
};

};

