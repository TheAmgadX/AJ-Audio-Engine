#include <iostream>
#include <chrono>
#include <cassert>
#include "../include/file_io/wav_file.h"

void print_audio_info(const AJ::AudioInfo& info) {
    std::cout << "AudioInfo:\n";
    std::cout << "  Length: " << info.length << '\n';
    std::cout << "  Samplerate: " << info.samplerate << '\n';
    std::cout << "  Channels: " << (int)info.channels << '\n';
    std::cout << "  Bitdepth: " << (int)info.bitdepth << '\n';
    std::cout << "  Format: " << info.format << '\n';
    std::cout << "  Seekable: " << info.seekable << '\n';
}

void test_wav_file(std::string path, const std::string& name, int expected_channels, int expected_bitdepth, int64_t expected_length) {
    std::cout << "Test: " << name << "\n";
    AJ::io::WAV_File wav;
    bool setPath = wav.setFilePath(path);
    bool setName = wav.setFileName(const_cast<std::string&>(name));
    assert(setPath);
    assert(setName);

    auto start = std::chrono::high_resolution_clock::now();
    bool result = wav.read();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Read result: " << result << ", Read time: " << elapsed.count() << " seconds\n";
    assert(result);
    print_audio_info(wav.mInfo);
    assert(wav.mInfo.channels == expected_channels);
    assert(wav.mInfo.bitdepth == expected_bitdepth);
    assert(wav.mInfo.length == expected_length);

    // Print number of blocks and some samples from each channel
    int num_blocks = (*wav.pAudio)[0].size();
    std::cout << "Number of blocks: " << num_blocks << "\n";
    for (int ch = 0; ch < wav.mInfo.channels; ++ch) {
        std::cout << "Channel " << ch << ":\n";
        for (int block = 0; block < num_blocks; ++block) {
            std::cout << "  Block " << block << ": ";
            for (int s = 0; s < 5 && s < (*wav.pAudio)[ch][block].size(); ++s) {
                std::cout << (*wav.pAudio)[ch][block][s] << " ";
            }
            std::cout << "...\n";
            if (block == 1) break; // Only print first 2 blocks per channel
        }
    }
    std::cout << name << " test passed.\n\n";
}

void test_wav_file_invalid(std::string path, const std::string& name) {
    std::cout << "Test: " << name << " (invalid/unsupported)\n";
    AJ::io::WAV_File wav;
    bool setPath = wav.setFilePath(path);
    bool setName = wav.setFileName(const_cast<std::string&>(name));
    // setFilePath may fail for invalid path, so only assert setName
    assert(setName);
    auto start = std::chrono::high_resolution_clock::now();
    bool result = wav.read();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Read result: " << result << ", Read time: " << elapsed.count() << " seconds\n";
    assert(!result);
    std::cout << name << " invalid/unsupported test passed.\n\n";
}
