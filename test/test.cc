// test.cc
#include <iostream>
#include <unistd.h>
#include "wav_file_tests.cc"

int main() {
    // Show current working directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        std::cout << "Current working directory: " << cwd << std::endl;
    }

    // Run WAV file tests
    WavFileTests::run_all();
    return 0;
}
