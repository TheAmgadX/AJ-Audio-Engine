#pragma once
#include <iostream>

#include "core/types.h"
#include "core/error_handler.h"


namespace AJ::io {

class AudioFile {
private:

    /// @brief check if the file extension available in the system from the path.
    /// @return return true if available false if not available.
    bool _available_file_extension(std::string ext) const; 

    AJ::FileExtension _stringToFileExtension(std::string ext);

    bool _validPath(String_c &path) const;

    bool _validDirectory(String_c &path) const;

    bool _trimFileName(std::string &name);

protected: 
    std::string mFileName;
    std::string mFilePath;
    AJ::FileExtension mExtension;
    AudioWriteInfo mWriteInfo;


public:
    /**
     * @brief Reads the audio file from disk into memory.
     *
     * This method is responsible for decoding the audio data and populating
     * the internal buffer. It must be implemented by derived classes according
     * to their specific file format (e.g., WAV, MP3).
     *
     * @param handler Reference to an error handler for reporting read issues.
     * @return true if the file was successfully read and decoded; false on failure.
     */
    virtual bool read(AJ::error::IErrorHandler &handler);

    /**
     * @brief Writes the audio data from memory to disk.
     *
     * This method encodes and saves the internal buffer to disk, using
     * format-specific logic implemented in the derived class.
     * Before calling this method, ensure that all write parameters are
     * properly set using `setWriteInfo()`.
     *
     * @param handler Reference to an error handler for reporting write issues.
     * @return true if the file was successfully written; false on failure.
     */
    virtual bool write(AJ::error::IErrorHandler &handler);

    ~AudioFile() = default;

    std::string FileName() const noexcept {
        return mFileName;
    }

    std::string FilePath() const noexcept {
        return mFilePath;
    }

    AJ::FileExtension FileExtension() const noexcept {
        return mExtension;
    }

    bool setFileName(std::string &name){
        if(_trimFileName(name)){
            mFileName = name;
            return true;
        }

        return false;
    }

    bool setFilePath(const std::string &path){
        if(_validPath(path)){
            mFilePath = path;
            return true;
        }

        return false;
    }

    bool setFileExtension(std::string ext){
        if(_available_file_extension(ext)){
            mExtension = _stringToFileExtension(ext);
            return true;
        }

        return false;
    }

    AudioFile(){
        pAudio = std::make_shared<AJ::AudioBuffer>();
    };

    /**
     * @brief Sets the necessary information for writing the audio file to disk.
     *
     * This method must be called before invoking `write()`. It configures details such as 
     * the output path, name, sample rate, number of channels, bit depth, whether the output should be seekable,
     * and target format.
     *
     * @param info Structure containing all required metadata for writing the file.
     * @param handler Reference to an error handler for reporting issues during setup.
     * @return true if the write information was successfully set; false if invalid parameters were provided.
     */
    bool setWriteInfo(const AJ::AudioWriteInfo& info, AJ::error::IErrorHandler &handler);

    AudioSamples pAudio;
    AudioInfo mInfo;
};
};