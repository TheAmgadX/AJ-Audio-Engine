// test.cc
#include <iostream>
#include <unistd.h>
#include "file_io/wav_file_tests.cc"
#include "file_io/mp3_file_tests.cc"
#include "file_io/file_streamer_tests.cc"

#include "echo/echo_tests.cc"
#include "gain/gain_tests.cc"
#include "reverb/reverb_tests.cc"
#include "fade/fade_tests.cc"
#include "normalization/norm_tests.cc"
#include "distortion/distortion_tests.cc"
#include "reverse/reverse_tests.cc"

#include "editing/cut/cut_tests.cc"
#include "editing/insert/insert_tests.cc"

#include "core/utils/buffer_pool_tests.cc"
#include "core/utils/ring_buffer_tests.cc"
#include "core/utils/thread_pool_tests.cc"

#include "audio_io/record_tests.cc"

int main() {
    // Show current working directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        std::cout << "Current working directory: " << cwd << std::endl;
    }
    
    // WavFileTests::run_all();

    // MP3FileTests::run_all();

    // EchoTests::run_all();

    // GainTests::run_all();

    // ReverbTests::run_all();

    // FadeTests::run_all();

    // NormalizationTests::run_all();

    // DistortionTests::run_all();
    
    // ReverseTests::run_all();

    // CutTests::run_all();

    // InsertTests::run_all();

    // BufferPoolTests::run_all();

    // RingBufferTests::run_all();


    // ThreadPoolTests::run_all();

    // FileStreamerWriteTests::run_all();

    AudioIOManagerRecordTests::run_all();

    return 0;
}
