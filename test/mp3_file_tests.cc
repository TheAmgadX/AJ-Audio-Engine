#include <iostream>
#include <chrono>
#include <cassert>
#include <filesystem>
#include "../include/file_io/mp3_file.h"
#include "../include/core/errors.h"
#include "../include/core/error_handler.h"

namespace fs = std::filesystem;

class MP3FileTests {
public:
    static void run_all() {
        std::cout << "\nRunning MP3 File Read/Write Tests\n";
        std::cout << "---------------------------------------------\n";
        test_valid_file("long_audio_stereo.mp3", 2);
        test_valid_file("medium_audio.mp3", 2);
        // test_valid_file("ogg_audio_file.ogg", 2); // TODO: fail when read any file before it.
        test_valid_file("flac_audio_file.flac", 2); //! work fine.
        test_valid_file("test_mp3.mp3", 1);
        // test_valid_file("long_audio.wav", 2);

        // test_invalid_file("does_not_exist.mp3");

        std::cout << "All tests completed.\n";
    }

private:
    static constexpr const char* audio_dir = "audio";
    static constexpr const char* output_dir = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/mp3_generated_audio";

    static void test_valid_file(const std::string& filename, int expected_channels) {
        using namespace AJ;
        using namespace AJ::io;
        using namespace AJ::error;

        ConsoleErrorHandler errorHandler;
        std::string input_path = std::string(audio_dir) + "/" + filename;

        MP3_File mp3 = MP3_File();
        if (!mp3.setFilePath(input_path)) {
            errorHandler.onError(Error::InvalidFilePath, "Failed to set file path: " + input_path);
            return;
        }

        if (!mp3.setFileName(const_cast<std::string&>(filename))) {
            errorHandler.onError(Error::InvalidFilePath, "Failed to set filename: " + filename);
            return;
        }

        auto start_read = std::chrono::high_resolution_clock::now();
        bool read_success = mp3.read(errorHandler);
        auto end_read = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_read = end_read - start_read;

        std::cout << "Test: " << filename << "\n";
        std::cout << "Read success: " << std::boolalpha << read_success << ", Time: " << elapsed_read.count() << "s\n";
        assert(read_success);

        const auto& info = mp3.mInfo;
        std::cout << "Length: " << info.length
                  << ", Channels: " << static_cast<int>(info.channels)
                  << ", Samplerate: " << info.samplerate << "\n";

        assert(info.channels == expected_channels);
        

        // Prepare write info (writes as WAV by default)
        AudioWriteInfo write_info;
        write_info.bitdepth = BitDepth_t::int_16; // any valid bit depth.
        write_info.channels = info.channels;
        write_info.length = info.length;
        write_info.samplerate = info.samplerate;
        write_info.seekable = true;
        write_info.path = std::string(output_dir);
        write_info.name = filename + "_converted";
        write_info.format = ".mp3";

        auto start_write = std::chrono::high_resolution_clock::now();

        if (!mp3.setWriteInfo(write_info, errorHandler)) {
            errorHandler.onError(Error::InvalidConfiguration, 
                "Failed to configure write settings for file: " + filename);
            return;
        }

        bool write_success = mp3.write(errorHandler);
        auto end_write = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_write = end_write - start_write;

        std::cout << "Write success: " << std::boolalpha << write_success
                  << ", Time: " << elapsed_write.count() << "s\n";
        assert(write_success);

        std::cout << "---------------------------------------------\n";
    }

    static void test_invalid_file(const std::string& filename) {
        using namespace AJ::io;
        using namespace AJ::error;

        ConsoleErrorHandler errorHandler;
        std::string input_path = std::string(audio_dir) + "/" + filename;

        MP3_File mp3;
        if (!mp3.setFilePath(input_path)) {
            errorHandler.onError(Error::InvalidFilePath, "Invalid file path: " + input_path);
            return;
        }

        if (!mp3.setFileName(const_cast<std::string&>(filename))) {
            errorHandler.onError(Error::InvalidFilePath, "Invalid filename: " + filename);
            return;
        }

        auto start = std::chrono::high_resolution_clock::now();
        bool result = mp3.read(errorHandler);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;

        std::cout << "Test: " << filename << " (Invalid/Unsupported)\n";
        std::cout << "Read result: " << std::boolalpha << result << ", Time: " << elapsed.count() << "s\n";

        if (result) {
            errorHandler.onError(Error::InternalError, "Expected read to fail for invalid file: " + filename);
        }

        std::cout << "---------------------------------------------\n";
    }
};
