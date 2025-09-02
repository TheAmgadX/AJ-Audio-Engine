#pragma once
#include <iostream>

#include "core/types.h"
#include "core/error_handler.h"
#include "file_io/file_utils.h"

namespace AJ::io {

/**
 * @brief Abstract base class for audio file handling (reading/writing).
 * 
 * This class provides common functionality for working with audio files and serves as
 * a base for format-specific implementations such as WAV and MP3. It manages metadata,
 * file paths, file names, and error handling.
 */
class AudioFile {
protected:
    std::string mFileName;     ///< File name including extension.
    std::string mFilePath;     ///< Full directory path (including file name).
    AudioWriteInfo mWriteInfo; ///< Information required for writing files.

public:
    AudioSamples pAudio;       ///< Pointer to multichannel audio buffer.
    AudioInfo mInfo;           ///< Metadata for the audio file (e.g., length, channels, sample rate).


    AudioFile() {
        pAudio = std::make_shared<AJ::AudioBuffer>();
    }

    virtual ~AudioFile() = default;

    /**
     * @brief Reads the audio file from disk into memory.
     * This is an offline reading method where the entire file is loaded into memory and remains
     *      there during processing.
     * 
     * This method must be implemented by derived classes. 
     * 
     * It decodes the file and populates `pAudio` and `mInfo`.
     * 
     * @param handler Reference to an error handler for reporting read issues.
     * @return true on success; false on failure.
     */
    virtual bool read(AJ::error::IErrorHandler &handler) = 0;

    /**
     * @brief Writes the audio buffer to disk using format-specific logic.
     * 
     * The `setWriteInfo()` method must be called before invoking this.
     * 
     * @param handler Reference to an error handler for reporting write issues.
     * @return true on success; false on failure.
     */
    virtual bool write(AJ::error::IErrorHandler &handler) = 0;

    /**
     * @brief Sets the necessary metadata for writing the file.
     * 
     * Validates and assigns write-related information like file name, sample rate, format, etc.
     * Only WAV and MP3 formats are supported.
     * 
     * @param info Audio write metadata.
     * @param handler Error handler for reporting validation issues.
     * @return true if the info is valid and accepted; false otherwise.
     */
    bool setWriteInfo(const AJ::AudioWriteInfo& info, AJ::error::IErrorHandler &handler);

    /**
     * @brief Retrieves the file name (with extension).
     * @return File name string.
     */
    std::string FileName() const noexcept {
        return mFileName;
    }

    /**
     * @brief Retrieves the file path (including file name).
     * @return File path string.
     */
    std::string FilePath() const noexcept {
        return mFilePath;
    }

    /**
     * @brief Sets the file name after trimming whitespace.
     * @param name File name string (should include extension).
     * @return true if valid and set; false otherwise.
     */
    bool setFileName(std::string &name) {
        if (AJ::utils::FileUtils::trim_file_name(name)) {
            mFileName = name;
            return true;
        }
        return false;
    }

    /**
     * @brief Sets the full file path after validation.
     * @param path Directory path + file name.
     * @return true if valid and set; false otherwise.
     */
    bool setFilePath(std::string &path) {
        if (AJ::utils::FileUtils::file_exists(path)) {
            mFilePath = path;
            return true;
        }
        return false;
    }
};

} // namespace AJ::io
