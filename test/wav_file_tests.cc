// wav_file_tests.cc
#include <iostream>
#include <chrono>
#include <cassert>
#include <filesystem>
#include "../include/file_io/wav_file.h"

namespace fs = std::filesystem;

class WavFileTests {
public:
    static void run_all() {
        std::cout << "\nRunning WAV File Read/Write Tests\n";
        std::cout << "---------------------------------------------\n";

        test_valid_file("long_audio.wav", 2, AJ::BitDepth_t::int_16);
        test_valid_file("test_24bit_stereo.wav", 2, AJ::BitDepth_t::int_24);
        test_valid_file("test_32bit_float_mono.wav", 1, AJ::BitDepth_t::float_32);
        test_valid_file("test_32bit_int_stereo.wav", 2, AJ::BitDepth_t::int_32);
        test_valid_file("test_64bit_double_mono.wav", 1, AJ::BitDepth_t::float_64);

        test_invalid_file("does_not_exist.wav");
        test_invalid_file("test_invalid.mp3");

        std::cout << "All tests completed.\n";
    }

private:
    static constexpr const char* audio_dir = "audio";
    static constexpr const char* output_dir = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/generated_audio";

    static const char* bitdepth_to_string(AJ::BitDepth_t depth) {
        using namespace AJ;
        switch (depth) {
            case BitDepth_t::int_8: return "8-bit PCM";
            case BitDepth_t::int_16: return "16-bit PCM";
            case BitDepth_t::int_24: return "24-bit PCM";
            case BitDepth_t::int_32: return "32-bit PCM";
            case BitDepth_t::float_32: return "32-bit Float";
            case BitDepth_t::float_64: return "64-bit Float";
            default: return "Not Supported";
        }
    }

    static void test_valid_file(const std::string& filename, int expected_channels, AJ::BitDepth_t expected_bitdepth) {
        using namespace AJ;
        using namespace AJ::io;

        std::string input_path = std::string(audio_dir) + "/" + filename;

        WAV_File wav;
        assert(wav.setFilePath(input_path));
        assert(wav.setFileName(const_cast<std::string&>(filename)));

        // Read
        auto start_read = std::chrono::high_resolution_clock::now();
        bool read_success = wav.read();
        auto end_read = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_read = end_read - start_read;

        std::cout << "Test: " << filename << "\n";
        std::cout << "Read success: " << std::boolalpha << read_success << ", Time: " << elapsed_read.count() << "s\n";
        assert(read_success);

        const auto& info = wav.mInfo;
        std::cout << "Length: " << info.length
                  << ", Channels: " << (int)info.channels
                  << ", Bitdepth: " << bitdepth_to_string(info.bitdepth)
                  << ", Samplerate: " << info.samplerate << "\n";

        assert(info.channels == expected_channels);
        assert(info.bitdepth == expected_bitdepth);

        // Now reuse the same object to write
        AudioWriteInfo write_info;
        write_info.bitdepth = info.bitdepth;
        write_info.channels = info.channels;
        write_info.length = info.length;
        write_info.samplerate = info.samplerate;
        write_info.seekable = true;
        write_info.path = std::string(output_dir);
        write_info.name = filename;
        write_info.format = ".wav";

        bool set_write_ok = wav.setWriteInfo(write_info);
        assert(set_write_ok);

        auto start_write = std::chrono::high_resolution_clock::now();
        bool write_success = wav.write();
        auto end_write = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_write = end_write - start_write;

        std::cout << "Write success: " << std::boolalpha << write_success
                  << ", Time: " << elapsed_write.count() << "s\n";
        assert(write_success);

        std::cout << "---------------------------------------------\n";
    }

    static void test_invalid_file(const std::string& filename) {
        using namespace AJ::io;
        WAV_File wav;
        std::string input_path = std::string(audio_dir) + "/" + filename;
        wav.setFilePath(input_path);
        wav.setFileName(const_cast<std::string&>(filename));

        auto start = std::chrono::high_resolution_clock::now();
        bool result = wav.read();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;

        std::cout << "Test: " << filename << " (Invalid/Unsupported)\n";
        std::cout << "Read result: " << std::boolalpha << result << ", Time: " << elapsed.count() << "s\n";
        assert(!result);
        std::cout << "---------------------------------------------\n";
    }
};
