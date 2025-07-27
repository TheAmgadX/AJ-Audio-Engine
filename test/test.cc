// test.cc
#include <iostream>
#include <unistd.h>
#include "wav_file_tests.cc"
#include "echo/echo_tests.cc"
#include "gain/gain_tests.cc"
#include "reverb/reverb_tests.cc"

int main() {
    // Show current working directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        std::cout << "Current working directory: " << cwd << std::endl;
    }

    // // Run WAV file tests
    // WavFileTests::run_all();

    // // Run Echo Tests
    // EchoTests::run_all();

    // // Run Gain Tests
    // GainTests::run_all();

    // // Run Reverb Tests
    // ReverbTests::run_all();

    return 0;
}
