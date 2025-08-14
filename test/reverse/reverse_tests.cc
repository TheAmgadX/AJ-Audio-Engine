#include <iostream>
#include <chrono>
#include <cassert>
#include <filesystem>

#include "dsp/reverse.h"
#include "file_io/wav_file.h"
#include "core/error_handler.h"

namespace fs = std::filesystem;

class ReverseTests {
public:
    static void run_all() {
        std::cout << "\nRunning Reverse Processing Tests (In-Place)\n";
        std::cout << "---------------------------------------------\n";

        test_reverse_on_valid_file("long_audio.wav", 2, "full");
        test_reverse_on_valid_file("test_24bit_stereo.wav", 2, "partial");
        test_reverse_on_valid_file("test_32bit_float_mono.wav", 1, "full");
        test_reverse_on_valid_file("reversed_audio.wav", 1, "full");
        test_reverse_with_invalid_indexes("test_32bit_int_stereo.wav", 2);
        test_reverse_on_valid_file("test_64bit_double_mono.wav", 1, "partial");

        std::cout << "All Reverse Tests Completed Successfully.\n";
    }

private:
    static constexpr const char* audio_dir  = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/audio";
    static constexpr const char* output_dir = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/reverse_audio";

    static void test_reverse_on_valid_file(const std::string& filename, short expected_channels, const std::string& mode) {
        using namespace AJ;
        using namespace AJ::io;
        using namespace AJ::dsp::reverse;

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
        std::cout << "\nTest: Reverse (in-place) on " << filename << " [" << mode << "]\n";
        std::cout << "Read Time: " << read_time.count() << "s\n";

        const auto& info = wav.mInfo;
        assert(info.channels == expected_channels);

        Reverse reverse;
        AudioSamples pAudio = wav.pAudio;
        assert(pAudio);

        // Compute selection (end is inclusive)
        sample_pos start = 0;
        sample_pos end   = (info.length / info.channels) - 1;

        if (mode == "partial") {
            // Reverse from 5s to the middle of the file
            start = 5 * info.samplerate;     // 5 seconds
            end   = end / 2;                 // halfway point
            if (start >= end) start = 0;     // guard: ensure valid range
        }

        Params paramsStruct{
            static_cast<sample_c>(start),
            static_cast<sample_c>(end)
        };

        auto params = ReverseParams::create(paramsStruct, errorHandler);
        assert(params);
        assert(reverse.setParams(params, errorHandler));

        auto process_start = std::chrono::high_resolution_clock::now();

        // Process each channel independently (planar buffers)
        reverse.process((*pAudio)[0], errorHandler);
        if (info.channels > 1) {
            reverse.process((*pAudio)[1], errorHandler);
        }

        auto process_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> process_time = process_end - process_start;
        std::cout << "Processing Time: " << process_time.count() << "s\n";

        // Prepare output write
        AudioWriteInfo write_info{
            info.length, info.samplerate, info.channels,
            info.bitdepth, ".wav", true,
            std::string(output_dir),
            "reverse_" + mode + "_" + filename
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

    static void test_reverse_with_invalid_indexes(const std::string& filename, short expected_channels) {
        using namespace AJ;
        using namespace AJ::io;
        using namespace AJ::dsp::reverse;

        error::ConsoleErrorHandler errorHandler;

        WAV_File wav;
        std::string input_path = std::string(audio_dir) + "/" + filename;
        assert(wav.setFilePath(input_path));
        assert(wav.setFileName(const_cast<std::string&>(filename)));
        assert(wav.read(errorHandler));

        std::cout << "\nTest: Reverse with Invalid Indexes on " << filename << "\n";

        const auto& info = wav.mInfo;
        assert(info.channels == expected_channels);

        AudioSamples pAudio = wav.pAudio;
        assert(pAudio);

        // Deliberately invalid: start beyond end (inclusive indexing)
        Params badParams{
            static_cast<sample_c>(info.length),      // start (invalid)
            static_cast<sample_c>(info.length / 2)   // end
        };

        auto params = ReverseParams::create(badParams, errorHandler);
        assert(!params); // Expect failure due to invalid range

        std::cout << "Handled invalid range without crashing.\n";
        std::cout << "---------------------------------------------\n";
    }
};
