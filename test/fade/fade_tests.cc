#include <iostream>
#include <chrono>
#include <cassert>
#include <filesystem>
#include <cmath>

#include "dsp/fade.h"
#include "file_io/wav_file.h"
#include "core/error_handler.h"

namespace fs = std::filesystem;

class FadeTests {
public:
    static void run_all() {
        std::cout << "\nRunning Fade Processing Tests (Auto SIMD/Naive)\n";
        std::cout << "---------------------------------------------\n";

        test_fade_on_valid_file("long_audio.wav", 2, AJ::dsp::FadeMode::In, 0.0f, 1.0f, "fade_in_full");
        test_fade_on_valid_file("test_24bit_stereo.wav", 2, AJ::dsp::FadeMode::Out, 0.0f, 1.0f, "fade_out_full");
        test_fade_on_valid_file("test_32bit_float_mono.wav", 1, AJ::dsp::FadeMode::In, 0.0f, 2.0f, "fade_in_partial", true);
        test_fade_on_valid_file("test_32bit_float_mono.wav", 1, AJ::dsp::FadeMode::In, 0.0f, 1.0f, "fade_in_full2");
        test_fade_with_invalid_indexes("test_32bit_int_stereo.wav", 2);

        std::cout << "All Fade Tests Completed Successfully.\n";
    }

private:
    static constexpr const char* audio_dir = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/audio";
    static constexpr const char* output_dir = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/fade_audio";

    static void test_fade_on_valid_file(
        const std::string& filename,
        short expected_channels,
        AJ::dsp::FadeMode mode,
        float lowGain,
        float highGain,
        const std::string& test_name,
        bool partial = false
    ) {
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
        std::cout << "\nTest: " << test_name << " on " << filename << "\n";
        std::cout << "Read Time: " << read_time.count() << "s\n";

        const auto& info = wav.mInfo;
        assert(info.channels == expected_channels);

        Fade fadeEffect;
        AudioSamples pAudio = wav.pAudio;
        assert(pAudio);

        sample_pos start = 0;
        sample_pos end = (info.length / info.channels) - 1;
        if (partial) {
            start = 5 * info.samplerate; // start from 5s
            end /= 2;
        }

        // Set parameters with your new design
        auto params = AJ::dsp::FadeParams::create(start, end, highGain, lowGain, mode, errorHandler);
        assert(params);
        assert(fadeEffect.setParams(params, errorHandler));

        auto process_start = std::chrono::high_resolution_clock::now();

        fadeEffect.process((*pAudio)[0], errorHandler);
        if (info.channels > 1) {
            fadeEffect.process((*pAudio)[1], errorHandler);
        }

        auto process_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> process_time = process_end - process_start;
        std::cout << "Processing Time: " << process_time.count() << "s\n";

        AudioWriteInfo write_info {
            info.length, info.samplerate, info.channels,
            info.bitdepth, ".wav", true,
            std::string(output_dir),
            test_name + "_" + filename
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

    static void test_fade_with_invalid_indexes(const std::string& filename, short expected_channels) {
        using namespace AJ;
        using namespace AJ::io;
        using namespace AJ::dsp;

        error::ConsoleErrorHandler errorHandler;

        WAV_File wav;
        std::string input_path = std::string(audio_dir) + "/" + filename;
        assert(wav.setFilePath(input_path));
        assert(wav.setFileName(const_cast<std::string&>(filename)));
        assert(wav.read(errorHandler));

        std::cout << "\nTest: Fade with Invalid Indexes on " << filename << "\n";

        const auto& info = wav.mInfo;
        assert(info.channels == expected_channels);

        Fade fadeEffect;
        AudioSamples pAudio = wav.pAudio;
        assert(pAudio);

        sample_pos start = info.length; // invalid
        sample_pos end = info.length / 2;

        float high = 1.0f, low = 0.0f;
        FadeMode mode = FadeMode::In;
        auto params = AJ::dsp::FadeParams::create(start, end, high, low, mode, errorHandler);
        assert(params);
        assert(fadeEffect.setParams(params, errorHandler));

        fadeEffect.process((*pAudio)[0], errorHandler);
        if (info.channels > 1) {
            fadeEffect.process((*pAudio)[1], errorHandler);
        }
        std::cout << "Handled invalid range without crashing.\n";
        std::cout << "---------------------------------------------\n";
    }
};
