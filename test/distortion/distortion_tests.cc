#include <iostream>
#include <chrono>
#include <cassert>
#include <filesystem>
#include <cmath>

#include "dsp/distortion.h"
#include "file_io/wav_file.h"
#include "core/error_handler.h"

namespace fs = std::filesystem;

class DistortionTests {
public:
    static void run_all() {
        std::cout << "\nRunning Distortion Processing Tests (SoftClipping)\n";
        std::cout << "--------------------------------------------------\n";

        // test_distortion_on_valid_file("long_audio.wav", 2, "full", 10.0f, AJ::dsp::distortion::DistortionType::SoftClipping);
        test_distortion_on_valid_file("test_24bit_stereo.wav", 2, "full", 5.0f, AJ::dsp::distortion::DistortionType::SoftClipping);
        test_distortion_on_valid_file("test_32bit_float_mono.wav", 1, "partial", 5.0f, AJ::dsp::distortion::DistortionType::SoftClipping);
        test_distortion_on_valid_file("test_64bit_double_mono.wav", 1, "full", 0.0f, AJ::dsp::distortion::DistortionType::SoftClipping);
        test_distortion_on_valid_file("guitar_short.wav", 2, "full", 4.0f, AJ::dsp::distortion::DistortionType::SoftClipping);

        test_distortion_with_invalid_indexes("test_32bit_int_stereo.wav", 2);

        std::cout << "All Distortion Tests Completed Successfully.\n";
    }

private:
    static constexpr const char* audio_dir  = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/audio";
    static constexpr const char* output_dir = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/dist_audio";

    static const char* typeToString(AJ::dsp::distortion::DistortionType t) {
        using AJ::dsp::distortion::DistortionType;
        switch (t) {
            case DistortionType::SoftClipping: return "softclip";
            default: return "unknown";
        }
    }

    static void test_distortion_on_valid_file(const std::string& filename,
                                              short expected_channels,
                                              const std::string& mode,
                                              float gain_value,
                                              AJ::dsp::distortion::DistortionType type)
    {
        using namespace AJ;
        using namespace AJ::io;
        using namespace AJ::dsp::distortion;

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
        std::cout << "\nTest: Distortion (" << typeToString(type) << ") on " << filename << "\n";
        std::cout << "Read Time: " << read_time.count() << "s\n";

        const auto& info = wav.mInfo;
        assert(info.channels == expected_channels);

        AJ::dsp::distortion::Distortion dist;
        AudioSamples pAudio = wav.pAudio;
        assert(pAudio);

        sample_c start = 0;
        sample_c end   = (info.length / info.channels) - 1; // inclusive
        if (mode == "partial") {
            start = static_cast<sample_c>(5) * static_cast<sample_c>(info.samplerate); // start from 5s
            end  /= 2;
        }

        auto process_start = std::chrono::high_resolution_clock::now();

        // Build params and create validated DistortionParams
        AJ::dsp::distortion::Params dParams;
        
        
        dParams.mStart = start;
        dParams.mEnd = end;
        dParams.mGain = gain_value;
        dParams.mType = type;


        auto params = DistortionParams::create(dParams, errorHandler);
        assert(params);
        assert(dist.setParams(params, errorHandler));

        // Process each channel in-place
        assert(pAudio->size() >= static_cast<size_t>(info.channels));
        for (int ch = 0; ch < info.channels; ++ch) {
            bool ok = dist.process((*pAudio)[ch], errorHandler);
            assert(ok);
        }

        auto process_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> process_time = process_end - process_start;
        std::cout << "Processing Time: " << process_time.count() << "s\n";

        AudioWriteInfo write_info {
            info.length, info.samplerate, info.channels,
            info.bitdepth, ".wav", true,
            std::string(output_dir),
            std::string("distortion_") + typeToString(type) + "_" + mode + "_" + filename
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

    static void test_distortion_with_invalid_indexes(const std::string& filename, short expected_channels) {
        using namespace AJ;
        using namespace AJ::io;
        using namespace AJ::dsp::distortion;

        error::ConsoleErrorHandler errorHandler;

        WAV_File wav;
        std::string input_path = std::string(audio_dir) + "/" + filename;
        assert(wav.setFilePath(input_path));
        assert(wav.setFileName(const_cast<std::string&>(filename)));
        assert(wav.read(errorHandler));

        std::cout << "\nTest: Distortion with Invalid Indexes on " << filename << "\n";

        const auto& info = wav.mInfo;
        assert(info.channels == expected_channels);

        AJ::dsp::distortion::Distortion dist;
        AudioSamples pAudio = wav.pAudio;
        assert(pAudio);

        // Invalid: start > end (start beyond buffer)
        AJ::dsp::distortion::Params dParams;
        
        dParams.mStart = info.length;
        dParams.mEnd = info.length / 2;
        dParams.mGain = 1.0f;
        dParams.mType = DistortionType::SoftClipping;

        auto params = DistortionParams::create(dParams, errorHandler);
        assert(!params); // Expected to fail validation

        std::cout << "Handled invalid range without crashing.\n";
        std::cout << "--------------------------------------------------\n";
    }
};
