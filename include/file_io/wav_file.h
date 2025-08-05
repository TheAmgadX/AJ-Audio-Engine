#pragma once
#include <iostream>
#include <vector>
#include <cstdint>
#include <fstream>
#include <string>
#include <sndfile.h> // docs: http://www.mega-nerd.com/libsndfile/api.html


#include "audio_file.h"
#include "core/types.h"


namespace AJ::io {

// TODO: support resampling using FFmpeg
/**
 * @class WAV_File
 * @brief Handles reading and writing WAV audio files using libsndfile.
 *
 * This class implements the AudioFile interface for `.wav` files,
 * supporting mono and stereo formats. It provides functionality for:
 * - Reading WAV files into memory (offline processing).
 * - Writing processed audio back to disk.
 *
 * @note Currently supports only mono and stereo audio formats.
 *
 * @see AJ::io::AudioFile
 * @see http://www.mega-nerd.com/libsndfile/api.html
 */
class WAV_File final : public io::AudioFile {
private:
    /**
     * @brief Safely closes the SNDFILE handle and optionally returns a success flag.
     * @param file Pointer to the open SNDFILE.
     * @param return_val Return value to propagate if close succeeds.
     * @param handler Error handler for reporting close failures.
     * @return false if closing failed, otherwise the value of `return_val`.
     */
    bool close_file(SNDFILE *file, bool return_val, AJ::error::IErrorHandler &handler);

    /**
     * @brief Extracts the bit depth from the SF_INFO struct.
     * @param info Reference to the file info returned by libsndfile.
     * @return Corresponding BitDepth_t enum value.
     */
    BitDepth_t get_bit_depth(const SF_INFO &info);

    /**
     * @brief Reads mono channel audio data into memory.
     * @param file Opened SNDFILE handle.
     * @param handler Error handler for reporting read failures.
     * @return true on success, false otherwise.
     */
    bool read_mono_data(SNDFILE *file, AJ::error::IErrorHandler &handler);

    /**
     * @brief Reads stereo channel audio data into memory.
     * @param file Opened SNDFILE handle.
     * @param handler Error handler for reporting read failures.
     * @return true on success, false otherwise.
     */
    bool read_stereo_data(SNDFILE *file, AJ::error::IErrorHandler &handler);

    /**
     * @brief Initializes SF_INFO struct using internal write info.
     *        Handles frame count, sample rate, seekability, and channels.
     * @param info Reference to SF_INFO struct to populate.
     */
    void set_file_info(SF_INFO &info);

    /**
     * @brief Writes mono channel audio data from memory to disk.
     * @param file Opened SNDFILE handle.
     * @param handler Error handler for reporting write failures.
     * @return true on success, false otherwise.
     */
    bool write_samples_mono(SNDFILE *file, AJ::error::IErrorHandler &handler);

    /**
     * @brief Writes stereo channel audio data from memory to disk.
     * @param file Opened SNDFILE handle.
     * @param handler Error handler for reporting write failures.
     * @return true on success, false otherwise.
     */
    bool write_samples_stereo(SNDFILE *file, AJ::error::IErrorHandler &handler);

public:
    /**
     * @brief Reads a WAV file and loads its contents into memory.
     * @param handler Error handler to report issues during reading.
     * @return true on successful read, false otherwise.
     */
    bool read(AJ::error::IErrorHandler &handler) override;

    /**
     * @brief Writes the internal audio buffer to a WAV file.
     * @param handler Error handler to report issues during writing.
     * @return true on successful write, false otherwise.
     */
    bool write(AJ::error::IErrorHandler &handler) override;
};

}