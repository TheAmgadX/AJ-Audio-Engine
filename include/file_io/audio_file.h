#pragma once
#include <iostream>

#include "core/types.h"
#include "core/error_handler.h"

namespace AJ::io {

/**
 * @brief Abstract base class for audio file handling (reading/writing).
 * 
 * This class provides common functionality for working with audio files and serves as
 * a base for format-specific implementations such as WAV and MP3. It manages metadata,
 * file paths, file names, and error handling.
 */
class AudioFile {
private:
    /**
     * @brief Checks whether the provided file extension is supported.
     * 
     * Only `.wav` and `.mp3` are currently supported.
     * 
     * @param ext The file extension (case-insensitive).
     * @return true if the extension is supported; false otherwise.
     */
    bool available_file_extension(std::string ext) const;

    /**
     * @brief Verifies that a given path points to a valid file.
     * 
     * @param path File path to check.
     * @return true if the file exists and is regular; false otherwise.
     */
    bool validPath(String_c &path) const;

    /**
     * @brief Verifies that the given path exists and is a directory.
     * 
     * @param path Directory path to check.
     * @return true if the directory exists; false otherwise.
     */
    bool validDirectory(String_c &path) const;

    /**
     * @brief Trims leading and trailing whitespace from a file name.
     * 
     * @param name Reference to the file name string.
     * @return true if the resulting name is non-empty; false otherwise.
     */
    bool trimFileName(std::string &name);

  
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
        if (trimFileName(name)) {
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
    bool setFilePath(const std::string &path) {
        if (validPath(path)) {
            mFilePath = path;
            return true;
        }
        return false;
    }

    /**
     * @brief Extract the file extension from the full path of the file.
     * 
     * @param path Reference to the full path which is directory + file name.
     * 
     * @return the extension if found in the file path; empty string otherwise.
     */
    std::string getFileExtension(const std::string& path);

};

} // namespace AJ::io
