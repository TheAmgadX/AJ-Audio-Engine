#include <iostream>
#include <cassert>
#include <unistd.h>

// Declarations for test functions in test_read_wav.cc
void test_wav_file(std::string path, const std::string& name, int expected_channels, int expected_bitdepth, int64_t expected_length);
void test_wav_file_invalid(std::string path, const std::string& name);

int main() {
    // Print the current working directory for reference
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        std::cout << "Current working directory: " << cwd << std::endl;
    }

    // NOTE:
    // Make sure the expected lengths match the actual files you have.
    // You can adjust these numbers as needed after confirming frame counts.

    // Test WAV files with correct expected lengths
    // (Update these values to match your actual files)
    //
    // If you generated or verified files yourself, adjust lengths accordingly.

    // Example: 36 seconds stereo 44100 Hz => frames = ~1,619,969, samples = 3,239,938
    test_wav_file("audio/test_16bit_stereo.wav", "test_16bit_stereo.wav", 2, 16, 3239938);

    // For other files below, adjust expected_length to match your files.
    // The numbers here are placeholders and should be verified.
    //
    // If you don't have these files yet, comment them out or create them with known duration.
    test_wav_file("audio/test_24bit_stereo.wav", "test_24bit_stereo.wav", 2, 24, 3239938);
    test_wav_file("audio/test_32bit_float_mono.wav", "test_32bit_float_mono.wav", 1, 32, 1619969);
    test_wav_file("audio/test_32bit_int_stereo.wav", "test_32bit_int_stereo.wav", 2, 32, 3239938);
    test_wav_file("audio/test_64bit_double_mono.wav", "test_64bit_double_mono.wav", 1, 64, 1619969);

    // Invalid or unsupported files
    test_wav_file_invalid("audio/does_not_exist.wav", "does_not_exist.wav");
    test_wav_file_invalid("audio/test_invalid.mp3", "test_invalid.mp3");

    std::cout << "All tests completed.\n";
    return 0;
}
