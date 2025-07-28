#include <iostream>
#include <chrono>
#include <cassert>
#include <filesystem>

#include "dsp/reverb/reverb.h"
#include "file_io/wav_file.h"
#include "core/error_handler.h"

namespace fs = std::filesystem;

class ReverbTests {
public:
    static void run_all() {
        std::cout << "\nRunning Reverb Processing Tests\n";
        std::cout << "---------------------------------------------\n";

        // test_reverb_on_valid_file("long_audio.wav", 2, "full", 70.0f, 0.0f, 1.0f, 0.8f);
        test_reverb_on_valid_file("test_16bit_stereo.wav", 2, "full",  25.0f, 0.7f, 0.3f, 0.4f);
        test_reverb_on_valid_file("test_32bit_float_mono.wav", 1, "partial", 40.0f, 0.2f, 0.8f, 0.9f);
        test_reverb_on_valid_file("test_64bit_double_mono.wav", 1, "full", 90.0f, 0.7f, 0.3f, 0.6f);
        test_reverb_on_valid_file("violin.wav", 2, "full", 25.0f, 0.7f, 0.3f, 0.4f);

        std::cout << "All Reverb Tests Completed Successfully.\n";
    }

private:
    static constexpr const char* audio_dir = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/audio";
    static constexpr const char* output_dir = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/reverb_audio";

    static void test_reverb_on_valid_file(const std::string& filename, short expected_channels, const std::string& mode,
                                          float delayMS, float dryMix, float wetMix, float gain) {
        using namespace AJ;
        using namespace AJ::io;
        using namespace AJ::dsp;
        using namespace AJ::dsp::reverb;

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
        std::cout << "\nTest: Reverb on " << filename << "\n";
        std::cout << "Read Time: " << read_time.count() << "s\n";

        const auto& info = wav.mInfo;
        assert(info.channels == expected_channels);

        Reverb reverb(info.samplerate);
        AudioSamples pAudio = wav.pAudio;
        assert(pAudio);

        // Calculate minimum required samples for reverb
        float total_delay_ms = delayMS + 89.27f + 19.31f;  // max delays
        sample_pos min_samples = static_cast<sample_pos>((total_delay_ms / 1000.0f) * info.samplerate * 2);
        
        // Check if file is long enough
        if (info.length / info.channels < min_samples) {
            std::cout << "Skipping file " << filename << ": too short for reverb settings\n";
            std::cout << "Need " << min_samples << " samples, have " << info.length / info.channels << "\n";
            return;
        }
        
        sample_pos start = 0;
        sample_pos end = (info.length / info.channels) - 1;
        if (mode == "partial") {
            start = 2 * info.samplerate; // 2 seconds offset
            // Ensure we leave enough samples for reverb tail
            end = std::min(start + (3 * info.samplerate), // 3 seconds of processing
                          (info.length / info.channels) - min_samples); // leave room for tail
        }

        ReverbParams params;
        params.mStart = start;
        params.mEnd = end;
        params.mDelayMS = delayMS;
        params.mDryMix = dryMix;
        params.mWetMix = wetMix;
        params.mGain = gain;
        params.mSamplerate = info.samplerate;
        assert(reverb.setParams(std::make_shared<ReverbParams>(params), errorHandler));

        auto process_start = std::chrono::high_resolution_clock::now();

        reverb.process((*pAudio)[0], errorHandler);
        if (info.channels > 1) {
            reverb.process((*pAudio)[1], errorHandler);
        }

        auto process_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> process_time = process_end - process_start;
        std::cout << "Processing Time: " << process_time.count() << "s\n";

        AudioWriteInfo write_info {
            info.length, info.samplerate, info.channels,
            info.bitdepth, ".wav", true,
            std::string(output_dir),
            "reverb_" + mode + "_" + filename
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
};
