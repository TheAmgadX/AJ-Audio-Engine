#include <iostream>
#include <chrono>
#include <cassert>
#include <filesystem>
#include <cmath>

#include "dsp/echo.h"
#include "file_io/wav_file.h"

namespace fs = std::filesystem;

class EchoNaiveTests {
public:
    static void run_all() {
        std::cout << "\nRunning Echo Naive Processing Tests\n";
        std::cout << "---------------------------------------------\n";

        test_echo_on_valid_file("long_audio.wav", 2);
        test_echo_on_valid_file("test_24bit_stereo.wav", 2);
        test_echo_on_valid_file("test_32bit_float_mono.wav", 1);
        test_echo_on_valid_file("test_32bit_int_stereo.wav", 2);
        test_echo_on_valid_file("test_64bit_double_mono.wav", 1);

        std::cout << "All Echo Naive Tests Completed Successfully.\n";
    }

private:
    static constexpr const char* audio_dir = "audio";
    static constexpr const char* output_dir = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/generated_echo_audio";

    static void test_echo_on_valid_file(const std::string& filename, short expected_channels) {
        using namespace AJ;
        using namespace AJ::io;
        using namespace AJ::dsp;

        std::string input_path = std::string(audio_dir) + "/" + filename;
        WAV_File wav;

        assert(wav.setFilePath(input_path));
        assert(wav.setFileName(const_cast<std::string&>(filename)));
        bool read_success = wav.read();
        assert(read_success);

        std::cout << "\nTest: Echo Naive on " << filename << "\n";
        const auto& info = wav.mInfo;
        assert(info.channels == expected_channels);

        // Setup echo
        Echo echo;
        echo.SetDecay(0.6f);
        echo.SetDelaySamples(0.2f, info.samplerate); // 200ms delay

        AudioSamples pAudio = wav.pAudio;
        assert(pAudio && "Audio buffer is null");

        sample_pos start = 0;
        sample_pos end = (info.length / 2)- 1;

        auto start_process = std::chrono::high_resolution_clock::now();
        echo.process(*pAudio, start, end, info.channels);
        auto end_process = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> elapsed = end_process - start_process;

        std::cout << "Processing complete (Naive), Time: " << elapsed.count() << "s\n";

        // Save processed buffer
        AudioWriteInfo write_info;
        write_info.bitdepth = info.bitdepth;
        write_info.channels = info.channels;
        write_info.length = info.length;
        write_info.samplerate = info.samplerate;
        write_info.seekable = true;
        write_info.path = std::string(output_dir);
        write_info.name = "echo_naive_" + filename;
        write_info.format = ".wav";

        bool write_info_ok = wav.setWriteInfo(write_info);
        assert(write_info_ok);

        bool write_success = wav.write();
        assert(write_success);

        std::cout << "Echo-processed file written to: " << output_dir << "/echo_naive_" << filename << "\n";
        std::cout << "---------------------------------------------\n";
    }
};
