#include <iostream>
#include <chrono>
#include <cassert>
#include <filesystem>
#include <cmath>

#include "dsp/echo.h"
#include "file_io/wav_file.h"
#include "core/error_handler.h"

namespace fs = std::filesystem;

class EchoTests {
public:
    static void run_all() {
        std::cout << "\nRunning Echo Processing Tests (Auto SIMD/Naive)\n";
        std::cout << "---------------------------------------------\n";

        test_echo_on_valid_file("long_audio.wav", 2, "full");
        test_echo_on_valid_file("test_24bit_stereo.wav", 2, "full");
        test_echo_on_valid_file("test_32bit_float_mono.wav", 1, "partial");
        test_echo_with_invalid_indexes("test_32bit_int_stereo.wav", 2);
        test_echo_on_valid_file("test_64bit_double_mono.wav", 1, "full");

        std::cout << "All Echo Tests Completed Successfully.\n";
    }

private:
    static constexpr const char* audio_dir = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/audio";
    static constexpr const char* output_dir = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/generated_echo_audio";

    static void test_echo_on_valid_file(const std::string& filename, short expected_channels, const std::string& mode) {
        using namespace AJ;
        using namespace AJ::io;
        using namespace AJ::dsp;

        std::string input_path = std::string(audio_dir) + "/" + filename;
        WAV_File wav;
        error::ConsoleErrorHandler errorHandler;

        assert(wav.setFilePath(input_path));
        assert(wav.setFileName(const_cast<std::string&>(filename)));

        auto read_start = std::chrono::high_resolution_clock::now();
        bool read_success = wav.read(errorHandler);
        auto read_end = std::chrono::high_resolution_clock::now();
        assert(read_success);

        std::chrono::duration<double> read_time = read_end - read_start;
        std::cout << "\nTest: Echo Auto (Naive or SIMD) on " << filename << "\n";
        std::cout << "Read Time: " << read_time.count() << "s\n";

        const auto& info = wav.mInfo;
        assert(info.channels == expected_channels);

        Echo echo;
        AudioSamples pAudio = wav.pAudio;
        assert(pAudio);

        sample_pos start = 0;
        sample_pos end = (info.length / info.channels) - 1;
        if (mode == "partial") {
            start = 5 * info.samplerate; // start from 5s
            end /= 2;
        }

        EchoParams params;
        params.mStart = start;
        params.mEnd = end;
        params.mDecay = 0.6f;
        params.mDelaySamples = 0.2f * info.samplerate;
        assert(echo.setParams(std::make_shared<EchoParams>(params), errorHandler));

        auto process_start = std::chrono::high_resolution_clock::now();
        
        echo.process((*pAudio)[0], errorHandler);
       
        if(info.channels > 1){
            echo.process((*pAudio)[1], errorHandler);
        }        

        auto process_end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> process_time = process_end - process_start;
        std::cout << "Processing Time: " << process_time.count() << "s\n";

        AudioWriteInfo write_info {
            info.length, info.samplerate, info.channels,
            info.bitdepth, ".wav", true,
            std::string(output_dir),
            "echo_auto_" + mode + "_" + filename
        };

        auto write_start = std::chrono::high_resolution_clock::now();
        assert(wav.setWriteInfo(write_info, errorHandler));
        assert(wav.write(errorHandler));
        auto write_end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> write_time = write_end - write_start;
        std::cout << "Write Time: " << write_time.count() << "s\n";

        std::cout << "Wrote: " << write_info.path << "/" << write_info.name << ".wav\n";
        std::cout << "---------------------------------------------\n";
    }

    static void test_echo_with_invalid_indexes(const std::string& filename, short expected_channels) {
        using namespace AJ;
        using namespace AJ::io;
        using namespace AJ::dsp;

        error::ConsoleErrorHandler errorHandler;
        WAV_File wav;
        std::string input_path = std::string(audio_dir) + "/" + filename;
        assert(wav.setFilePath(input_path));
        assert(wav.setFileName(const_cast<std::string&>(filename)));
        assert(wav.read(errorHandler));

        std::cout << "\nTest: Echo with Invalid Indexes on " << filename << "\n";

        const auto& info = wav.mInfo;
        assert(info.channels == expected_channels);

        Echo echo;
        echo.SetDecay(0.8f);
        echo.SetDelaySamples(0.4f, info.samplerate);

        AudioSamples pAudio = wav.pAudio;
        assert(pAudio);

        sample_pos start = info.length;
        sample_pos end = info.length / 2;

        echo.process((*pAudio)[0], errorHandler);
       
        if(info.channels > 1){
            echo.process((*pAudio)[1], errorHandler);
        }
        std::cout << "Handled invalid range without crashing.\n";
        std::cout << "---------------------------------------------\n";
    }
};