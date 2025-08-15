#include <iostream>
#include <chrono>
#include <cassert>
#include <filesystem>
#include <cmath>

#include "editing/cut.h"
#include "file_io/wav_file.h"
#include "core/error_handler.h"

namespace fs = std::filesystem;

class CutTests {
public:
    static void run_all() {
        std::cout << "\nRunning Cut Processing Tests\n";
        std::cout << "---------------------------------------------\n";

        // Format mirrors GainTests: filename, expected channels, mode, cut length in seconds
        test_cut_on_valid_file("long_audio.wav", 2, "full", 1.5f);
        test_cut_on_valid_file("test_24bit_stereo.wav", 2, "full", 0.5f);
        test_cut_on_valid_file("test_32bit_float_mono.wav", 1, "partial", 10.0f);
        test_cut_with_invalid_indexes("test_32bit_int_stereo.wav", 2);
        test_cut_on_valid_file("test_64bit_double_mono.wav", 1, "start", 0.1f);

        std::cout << "All Cut Tests Completed Successfully.\n";
    }

private:
    static constexpr const char* audio_dir = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/audio";
    static constexpr const char* output_dir = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/cut_audio";

    /**
     * @param filename - source wav filename
     * @param expected_channels - expected channel count (1 or 2)
     * @param mode - "partial" (cut from middle), "start" (cut from start), "full" (cut near start), "end" (cut near end)
     * @param cut_seconds - duration of cut in seconds (rounded to samples). If <= 0 the test skips processing.
     */
    static void test_cut_on_valid_file(const std::string& filename, short expected_channels, const std::string& mode, float cut_seconds) {
        using namespace AJ;
        using namespace AJ::io;
        using namespace AJ::editing::cut;

        std::string input_path = std::string(audio_dir) + "/" + filename;

        auto wav = std::make_shared<WAV_File>();  // ✅ shared_ptr from start
        error::ConsoleErrorHandler errorHandler;

        assert(wav->setFilePath(input_path));
        assert(wav->setFileName(const_cast<std::string&>(filename)));

        auto read_start = std::chrono::high_resolution_clock::now();
        bool read_success = wav->read(errorHandler);
        auto read_end = std::chrono::high_resolution_clock::now();
        assert(read_success);

        std::chrono::duration<double> read_time = read_end - read_start;
        std::cout << "\nTest: Cut on " << filename << " (mode=" << mode << ", cut=" << cut_seconds << "s)\n";
        std::cout << "Read Time: " << read_time.count() << "s\n";

        const auto& info = wav->mInfo;
        assert(info.channels == expected_channels);

        AudioSamples pAudio = wav->pAudio;
        assert(pAudio);

        sample_pos frames = info.length / info.channels;

        sample_pos start = 0;
        sample_pos end = 0;

        sample_pos cut_samples = static_cast<sample_pos>(std::llround(cut_seconds * static_cast<double>(info.samplerate)));
        if (cut_samples <= 0) {
            std::cout << "Cut length <= 0s, skipping processing (no-op test).\n";
            std::cout << "---------------------------------------------\n";
            return;
        }

        if (mode == "partial") {
            start = 5 * info.samplerate;
            if (start >= frames) start = frames / 4;
            end = start + cut_samples - 1;
            if (end >= frames) end = frames - 1;
        } else if (mode == "start" || mode == "full") {
            start = 0;
            end = frames - 1;
        } else if (mode == "end") {
            if (frames <= cut_samples) {
                start = 0;
            } else {
                start = frames - cut_samples;
            }
            end = frames - 1;
        } else {
            start = 5 * info.samplerate;
            if (start >= frames) start = 0;
            end = start + cut_samples - 1;
            if (end >= frames) end = frames - 1;
        }

        if (start > end || start < 0 || end < 0) {
            std::cerr << "Computed invalid cut range: start=" << start << " end=" << end << " frames=" << frames << "\n";
            assert(false && "Invalid computed cut range");
        }

        Cut cutter;
        bool range_ok = cutter.setRange(static_cast<sample_c>(start), static_cast<sample_c>(end), errorHandler);
        assert(range_ok);

        auto process_start = std::chrono::high_resolution_clock::now();

        bool proc_ok = cutter.process(wav, errorHandler);  // ✅ directly pass shared_ptr
        assert(proc_ok);

        auto process_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> process_time = process_end - process_start;
        std::cout << "Processing Time: " << process_time.count() << "s\n";

        sample_pos new_frames = static_cast<sample_pos>(wav->pAudio->at(0).size());
        sample_pos new_length = new_frames * info.channels;

        std::cout << "Original frames/channel: " << frames << ", New frames/channel: " << new_frames << "\n";

        AudioWriteInfo write_info {
            new_length,
            info.samplerate,
            info.channels,
            info.bitdepth,
            ".wav",
            true,
            std::string(output_dir),
            "cut_" + mode + "_" + filename
        };

        auto write_start = std::chrono::high_resolution_clock::now();
        assert(wav->setWriteInfo(write_info, errorHandler));
        assert(wav->write(errorHandler));
        auto write_end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> write_time = write_end - write_start;
        std::cout << "Write Time: " << write_time.count() << "s\n";
        std::cout << "Wrote: " << write_info.path << "/" << write_info.name << ".wav\n";
        std::cout << "---------------------------------------------\n";
    }

    static void test_cut_with_invalid_indexes(const std::string& filename, short expected_channels) {
        using namespace AJ;
        using namespace AJ::io;
        using namespace AJ::editing::cut;

        error::ConsoleErrorHandler errorHandler;

        auto wav = std::make_shared<WAV_File>();  // ✅
        std::string input_path = std::string(audio_dir) + "/" + filename;
        assert(wav->setFilePath(input_path));
        assert(wav->setFileName(const_cast<std::string&>(filename)));
        assert(wav->read(errorHandler));

        std::cout << "\nTest: Cut with Invalid Indexes on " << filename << "\n";

        const auto& info = wav->mInfo;
        assert(info.channels == expected_channels);

        AudioSamples pAudio = wav->pAudio;
        assert(pAudio);

        sample_pos frames = info.length / info.channels;

        Cut cutter;

        sample_c bad_start = static_cast<sample_c>(frames);
        sample_c bad_end   = static_cast<sample_c>(frames / 2);
        bool ok = cutter.setRange(bad_start, bad_end, errorHandler);
        assert(!ok);

        bool ok2 = cutter.setRange(static_cast<sample_c>(-5), static_cast<sample_c>(10), errorHandler);
        assert(!ok2);

        std::cout << "Handled invalid range cases without crashing.\n";
        std::cout << "---------------------------------------------\n";
    }

};
