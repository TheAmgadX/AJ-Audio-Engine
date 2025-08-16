// test.cc
#include <iostream>
#include <unistd.h>
#include "wav_file_tests.cc"
#include "mp3_file_tests.cc"

#include "echo/echo_tests.cc"
#include "gain/gain_tests.cc"
#include "reverb/reverb_tests.cc"
#include "fade/fade_tests.cc"
#include "normalization/norm_tests.cc"
#include "distortion/distortion_tests.cc"
#include "reverse/reverse_tests.cc"

#include "editing/cut/cut_tests.cc"
#include "editing/insert/insert_tests.cc"

int main() {
    // Show current working directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        std::cout << "Current working directory: " << cwd << std::endl;
    }

    // // Run MP3 file tests
    // MP3FileTests::run_all();

    // // Run WAV file tests
    // WavFileTests::run_all();

    // // Run Echo Tests
    // EchoTests::run_all();

    // // Run Gain Tests
    // GainTests::run_all();

    // // Run Reverb Tests
    // ReverbTests::run_all();

    // // Run Fade Tests
    // FadeTests::run_all();

    // // Run Normalization Tests
    // NormalizationTests::run_all();

    // // Run Distortion Tests
    // DistortionTests::run_all();
    
    // // Run Reverse Tests
    // ReverseTests::run_all();

    // // Run Cut Tests
    // CutTests::run_all();

    // Run Insert Tests
    InsertTests::run_all();

    return 0;
}
