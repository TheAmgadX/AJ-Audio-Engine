#include <iostream>
#include <chrono>
#include <cassert>
#include <filesystem>
#include <cmath>

#include "dsp/normalization.h"
#include "file_io/wav_file.h"
#include "core/error_handler.h"

namespace fs = std::filesystem;

class NormalizationTests {
public:
    static void run_all() {
        std::cout << "\nRunning Normalization Processing Tests (Peak & RMS)\n";
        std::cout << "--------------------------------------------------\n";

        // Peak normalization - full file
        test_normalization("long_audio.wav", 2, "full", AJ::dsp::NormalizationMode::Peak, 1.0f);
        
        // RMS normalization - full file
        test_normalization("test_24bit_stereo.wav", 2, "full", AJ::dsp::NormalizationMode::RMS, 0.5f);

        // Peak normalization - partial segment
        test_normalization("test_32bit_float_mono.wav", 1, "partial", AJ::dsp::NormalizationMode::Peak, 0.5f);

        // RMS normalization - partial segment
        test_normalization("test_32bit_int_stereo.wav", 2, "partial", AJ::dsp::NormalizationMode::RMS, 0.9f);

        // Factor beyond allowed range (should clamp internally)
        test_normalization("test_64bit_double_mono.wav", 1, "full", AJ::dsp::NormalizationMode::Peak, 0.2f);

        // Invalid range test
        // test_normalization_invalid_range("long_audio.wav", 2);

        std::cout << "All Normalization Tests Completed Successfully.\n";
    }

private:
    static constexpr const char* audio_dir =
        "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/audio";
    static constexpr const char* output_dir =
        "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/norm_audio";

    static void test_normalization(const std::string& filename,
                                   short expected_channels,
                                   const std::string& mode,
                                   AJ::dsp::NormalizationMode norm_mode,
                                   float factor) {
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
        std::cout << "\nTest: Normalization (" 
                  << (norm_mode == NormalizationMode::Peak ? "Peak" : "RMS")
                  << ") on " << filename << "\n";
        std::cout << "Read Time: " << read_time.count() << "s\n";

        const auto& info = wav.mInfo;
        assert(info.channels == expected_channels);

        AudioSamples pAudio = wav.pAudio;
        assert(pAudio);

        sample_pos start = 0;
        sample_pos end = (info.length / info.channels) - 1;
        if (mode == "partial") {
            start = 5 * info.samplerate; // start at 5s
            end /= 2;
        }

        auto params = NormalizationParams::create(start, end, errorHandler, factor, norm_mode);
        assert(params);

        Normalization normalization(params, errorHandler);

        auto process_start = std::chrono::high_resolution_clock::now();
        normalization.process((*pAudio)[0], errorHandler);
        if (info.channels > 1) {
            normalization.process((*pAudio)[1], errorHandler);
        }
        auto process_end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> process_time = process_end - process_start;
        std::cout << "Processing Time: " << process_time.count() << "s\n";

        AudioWriteInfo write_info {
            info.length, info.samplerate, info.channels,
            info.bitdepth, ".wav", true,
            std::string(output_dir),
            "norm_" + (norm_mode == NormalizationMode::Peak ? std::string("peak") : std::string("rms"))
                     + "_" + mode + "_" + filename
        };

        auto write_start = std::chrono::high_resolution_clock::now();
        assert(wav.setWriteInfo(write_info, errorHandler));
        assert(wav.write(errorHandler));
        auto write_end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> write_time = write_end - write_start;
        std::cout << "Write Time: " << write_time.count() << "s\n";
        std::cout << "Wrote: " << write_info.path << "/" << write_info.name << ".wav\n";
        std::cout << "--------------------------------------------------\n";
    }

    static void test_normalization_invalid_range(const std::string& filename,
                                                 short expected_channels) {
        using namespace AJ;
        using namespace AJ::io;
        using namespace AJ::dsp;

        error::ConsoleErrorHandler errorHandler;
        WAV_File wav;

        std::string input_path = std::string(audio_dir) + "/" + filename;
        assert(wav.setFilePath(input_path));
        assert(wav.setFileName(const_cast<std::string&>(filename)));
        assert(wav.read(errorHandler));

        std::cout << "\nTest: Normalization with Invalid Indexes on " << filename << "\n";

        const auto& info = wav.mInfo;
        assert(info.channels == expected_channels);

        AudioSamples pAudio = wav.pAudio;
        assert(pAudio);

        // Invalid range: start after end
        sample_pos start = info.length;
        sample_pos end = info.length / 2;

        auto params = NormalizationParams::create(start, end, errorHandler, 1.0f, NormalizationMode::RMS);
        assert(!params && "Expected failure due to invalid range");

        std::cout << "Handled invalid range without crashing.\n";
        std::cout << "--------------------------------------------------\n";
    }
};
