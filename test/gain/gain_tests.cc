#include <iostream>
#include <chrono>
#include <cassert>
#include <filesystem>
#include <cmath>

#include "dsp/gain.h"
#include "file_io/wav_file.h"
#include "core/error_handler.h"

namespace fs = std::filesystem;

class GainTests {
public:
    static void run_all() {
        std::cout << "\nRunning Gain Processing Tests (Auto SIMD/Naive)\n";
        std::cout << "---------------------------------------------\n";

        test_gain_on_valid_file("long_audio.wav", 2, "full", 1.5f);
        test_gain_on_valid_file("test_24bit_stereo.wav", 2, "full", 0.5f);
        test_gain_on_valid_file("test_32bit_float_mono.wav", 1, "partial", 2.0f);
        test_gain_with_invalid_indexes("test_32bit_int_stereo.wav", 2);
        test_gain_on_valid_file("test_64bit_double_mono.wav", 1, "full", 0.0f);

        std::cout << "All Gain Tests Completed Successfully.\n";
    }

private:
    static constexpr const char* audio_dir = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/audio";
    static constexpr const char* output_dir = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/gain_audio";

    static void test_gain_on_valid_file(const std::string& filename, short expected_channels, const std::string& mode, float gain_value) {
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
        std::cout << "\nTest: Gain Auto (Naive or SIMD) on " << filename << "\n";
        std::cout << "Read Time: " << read_time.count() << "s\n";

        const auto& info = wav.mInfo;
        assert(info.channels == expected_channels);

        Gain gain;
        assert(gain.setGain(gain_value, errorHandler));

        AudioSamples pAudio = wav.pAudio;
        assert(pAudio);

        sample_pos start = 0;
        sample_pos end = (info.length / info.channels) - 1;
        if (mode == "partial") {
            start = 5 * info.samplerate; // start from 5s
            end /= 2;
        }

        auto process_start = std::chrono::high_resolution_clock::now();
        
        gain.process((*pAudio)[0], start, end, errorHandler);
       
        if(info.channels > 1){
            gain.process((*pAudio)[1], start, end, errorHandler);
        }

        auto process_end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> process_time = process_end - process_start;
        std::cout << "Processing Time: " << process_time.count() << "s\n";

        AudioWriteInfo write_info {
            info.length, info.samplerate, info.channels,
            info.bitdepth, ".wav", true,
            std::string(output_dir),
            "gain_auto_" + mode + "_" + filename
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

    static void test_gain_with_invalid_indexes(const std::string& filename, short expected_channels) {
        using namespace AJ;
        using namespace AJ::io;
        using namespace AJ::dsp;

        error::ConsoleErrorHandler errorHandler;


        WAV_File wav;
        std::string input_path = std::string(audio_dir) + "/" + filename;
        assert(wav.setFilePath(input_path));
        assert(wav.setFileName(const_cast<std::string&>(filename)));
        assert(wav.read(errorHandler));

        std::cout << "\nTest: Gain with Invalid Indexes on " << filename << "\n";

        const auto& info = wav.mInfo;
        assert(info.channels == expected_channels);

        Gain gain;
        assert(gain.setGain(1.0f, errorHandler));

        AudioSamples pAudio = wav.pAudio;
        assert(pAudio);

        sample_pos start = info.length; // invalid
        sample_pos end = info.length / 2;

        gain.process((*pAudio)[0],start, end, errorHandler);
       
        if(info.channels > 1){
            gain.process((*pAudio)[1],start, end, errorHandler);
        }
        std::cout << "Handled invalid range without crashing.\n";
        std::cout << "---------------------------------------------\n";
    }
};
