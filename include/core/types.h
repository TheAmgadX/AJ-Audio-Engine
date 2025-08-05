#pragma once

#include <vector>
#include <array>
#include <cstdint>
#include <memory>

#include "constants.h"

namespace AJ {

/// @brief Gain value (linear scale).
typedef float gain_t;

/// @brief Decay value used in effects like echo or reverb.
typedef float decay_t;

/// @brief Single audio sample (float PCM).
typedef float sample_t;

/// @brief Sample count across buffer or file.
typedef int64_t sample_c;

/// @brief Sample position/index in a stream.
typedef int64_t sample_pos;

/// @brief Single-channel buffer of float samples.
using Float = std::vector<float>;

/// @brief Multichannel audio buffer organized by processing blocks.
/// Outer array: channels, inner vectors: sample blocks.
using AudioBuffer = std::array<Float, kNumChannels>;

/// @brief Shared pointer to audio sample data (used in AudioFile).
using AudioSamples = std::shared_ptr<AudioBuffer>;

/// @brief Read-only string reference.
using String_c = const std::string;


/**
 * @brief Enumerates supported bit depths for WAV files.
 *
 * This enum defines the sample formats handled specifically by the WAV_File class,
 * which uses libsndfile for reading and writing WAV data. Bit depths listed here
 * determine the precision of each sample stored in the audio file.
 */
enum BitDepth_t{
    int_8,
    int_16,
    int_24,
    int_32,
    float_32,
    float_64,
    Not_Supported
};

/**
 * @brief Stores metadata about an audio file.
 *
 * This structure is used to hold audio format and stream information
 * for each AudioFile instance (stored in the mInfo member).
 */
struct AudioInfo {
    /**
     * @brief Total number of samples across all channels.
     */
    sample_c length;

    /**
     * @brief Sampling rate in Hz (e.g., 44100, 48000).
     */
    sample_c samplerate;

    /**
     * @brief Number of audio channels (e.g., 1 for mono, 2 for stereo).
     */
    uint8_t channels;

    /**
     * @brief Bit depth of the audio stream.
     *
     * Used only for WAV_File class. Not applicable to files decoded via FFmpeg like MP3_File class.
     */
    BitDepth_t bitdepth;

    /**
     * @brief Format name or file type (e.g., ".wav", ".mp3").
     */
    std::string format;

    /**
     * @brief Indicates whether the audio stream supports seeking.
     *
     * This is only meaningful for WAV files. FFmpeg-based functionalities may not set this.
     */
    bool seekable;
};

/**
 * @brief Holds information required for writing an audio file.
 *
 * This structure is used to configure audio file output, including format,
 * metadata, and path-related information. It is typically passed to file writing
 * components to specify how and where to write the audio data.
 */
struct AudioWriteInfo {
    /**
     * @brief Total number of samples across all channels.
     */
    sample_c length;

    /**
     * @brief Sampling rate in Hz (e.g., 44100, 48000).
     */
    sample_c samplerate;

    /**
     * @brief Number of audio channels (e.g., 1 for mono, 2 for stereo).
     */
    uint8_t channels;

    /**
     * @brief Bit depth of the output file.
     *
     * Used only for WAV_File class. Not applicable to files decoded via FFmpeg like MP3_File class.
     */
    BitDepth_t bitdepth;

    /**
     * @brief Target output format (e.g., "wav", "mp3").
     */
    std::string format;

    /**
     * @brief Whether the output file should support seeking.
     *
     * This is relevant mostly for WAV_File class.
     */
    bool seekable;

    /**
     * @brief Full path where the file will be written (directory only).
     * The full output path is constructed in the writing function as: `path + "/" + name`.
     */
    std::string path;

    /**
     * @brief Name of the audio file (without path).
     * This must include the file extension (e.g., "output.wav", "track.mp3").
     */
    std::string name;
};


/**
 * @brief Enumerates the supported DSP (Digital Signal Processing) effects.
 *
 * This enum defines the list of audio effects that can be applied to audio buffers
 * using the applyEffect() functions. Each effect corresponds to a specific type of
 * signal processing operation.
 */
enum Effect {
    /**
     * @brief Applies a distortion effect, introducing harmonic saturation or clipping.
     */
    Distortion,

    /**
     * @brief Applies an echo effect by adding delayed repetitions of the signal.
     */
    echo,

    /**
     * @brief Applies a reverb effect to simulate spatial acoustics and room reflections.
     */
    reverb,

    /**
     * @brief Gradually increases the volume from silence to full level.
     */
    fadeIn,

    /**
     * @brief Gradually decreases the volume from full level to silence.
     */
    fadeOut,

    /**
     * @brief Adjusts the overall amplitude of the audio signal.
     */
    gain,

    /**
     * @brief Normalizes the signal so its peak or RMS level reaches a target value.
     */
    normalization,

    /**
     * @brief Changes the pitch of the audio without affecting its duration.
     */
    pitchShift,

    /**
     * @brief Reverses the audio data in time.
     */
    reverse
};


};

