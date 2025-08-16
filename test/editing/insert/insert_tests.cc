#include <iostream>
#include <chrono>
#include <cassert>
#include <filesystem>

#include "editing/insert.h"
#include "file_io/wav_file.h"
#include "core/error_handler.h"
#include "core/types.h"

namespace fs = std::filesystem;

class InsertTests {
public:
    static void run_all() {
        std::cout << "\nRunning Insert Processing Tests\n";
        std::cout << "---------------------------------------------\n";

        test_insert_at_position("long_audio.wav", "guitar_short.wav", 2, "beginning", 0);
        test_insert_at_position("long_audio.wav", "guitar_short.wav", 2, "middle", -1);
        test_insert_at_position("long_audio.wav", "guitar_short.wav", 2, "end", -2);

        std::cout << "All Insert Tests Completed Successfully.\n";
    }

private:
    static constexpr const char* audio_dir  = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/audio";
    static constexpr const char* output_dir = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/insert_audio";

    static void test_insert_at_position(const std::string& target_filename,
                                        const std::string& insert_filename,
                                        short expected_channels,
                                        const std::string& mode,
                                        AJ::sample_c insert_index) {
        using namespace AJ;
        using namespace AJ::io;
        using namespace AJ::editing::insert;

        error::ConsoleErrorHandler errorHandler;

        // --- Load target file into shared_ptr ---
        auto targetWav = std::make_shared<WAV_File>();
        std::string input_path = std::string(audio_dir) + "/" + target_filename;
        assert(targetWav->setFilePath(input_path));
        assert(targetWav->setFileName(const_cast<std::string&>(target_filename)));

        auto read_start = std::chrono::high_resolution_clock::now();
        assert(targetWav->read(errorHandler));
        auto read_end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> read_time = read_end - read_start;
        std::cout << "\nTest: Insert (" << mode << ") on " << target_filename << "\n";
        std::cout << "Read Time (Target): " << read_time.count() << "s\n";

        const auto& info = targetWav->mInfo;
        assert(info.channels == expected_channels);

        // --- Load insert file ---
        auto insertWav = std::make_shared<WAV_File>();
        std::string insert_path = std::string(audio_dir) + "/" + insert_filename;
        assert(insertWav->setFilePath(insert_path));
        assert(insertWav->setFileName(const_cast<std::string&>(insert_filename)));
        assert(insertWav->read(errorHandler));
        assert(insertWav->pAudio);

        // --- Insert logic ---
        Insert inserter;

        sample_c finalIndex = insert_index;
        if (insert_index == -1) {
            finalIndex = (info.length / info.channels) / 2; // middle
        } else if (insert_index == -2) {
            finalIndex = (info.length / info.channels);     // end
        }

        assert(inserter.setInsertAt(finalIndex, errorHandler));

        auto process_start = std::chrono::high_resolution_clock::now();
        bool success = inserter.process(
            targetWav,          // SAME shared_ptr throughout
            insertWav->pAudio,
            errorHandler
        );
        auto process_end = std::chrono::high_resolution_clock::now();
        assert(success);

        std::chrono::duration<double> process_time = process_end - process_start;
        std::cout << "Processing Time: " << process_time.count() << "s\n";

        // --- Write output ---
        AudioWriteInfo write_info {
            targetWav->mInfo.length,
            targetWav->mInfo.samplerate,
            targetWav->mInfo.channels,
            targetWav->mInfo.bitdepth,
            ".wav",
            true,
            std::string(output_dir),
            "insert_" + mode + "_" + target_filename
        };

        auto write_start = std::chrono::high_resolution_clock::now();
        assert(targetWav->setWriteInfo(write_info, errorHandler));
        assert(targetWav->write(errorHandler));
        auto write_end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> write_time = write_end - write_start;
        std::cout << "Write Time: " << write_time.count() << "s\n";
        std::cout << "Wrote: " << write_info.path << "/" << write_info.name << ".wav\n";
        std::cout << "---------------------------------------------\n";
    }
};
