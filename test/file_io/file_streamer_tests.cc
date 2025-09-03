#include <iostream>
#include <thread>
#include <vector>
#include <cassert>
#include <atomic>
#include <sndfile.h>

#include "file_io/file_streamer.h"
#include "core/buffer_pool.h"
#include "core/error_handler.h"
#include "core/thread_pool.h"

class FileStreamerWriteTests {
public:
    static void run_all() {
        std::cout << "\nRunning FileStreamer Write Tests\n";
        std::cout << "---------------------------------------------\n";

        test_multi_buffer_write();

        std::cout << "All FileStreamer Write Tests Completed Successfully.\n";
    }

private:
    static void test_multi_buffer_write() {
        std::cout << "\nTest: Multi-threaded Buffer Write\n";
        AJ::error::ConsoleErrorHandler handler;

        // Test parameters
        const size_t queue_capacity = 1024;
        const size_t buffer_frames = 1024;
        const uint8_t channels = 1; // mono
        const size_t frame_samples = buffer_frames * channels;
        const std::chrono::seconds test_duration(5);
        const std::chrono::milliseconds producer_sleep(20);

        // Pool + Queue + StopFlag
        auto pool = std::make_shared<AJ::utils::BufferPool>(handler, queue_capacity,
                                                            buffer_frames, channels);

        auto queue = std::make_shared<AJ::utils::Queue>(/*empty=*/true,
                                                        queue_capacity,
                                                        buffer_frames,
                                                        channels,
                                                        handler);
                                                        
        auto stopFlag = std::make_shared<std::atomic<bool>>(false);

        assert(pool->isValid());
        assert(queue->isValid());

        // FileStreamer setup
        std::string sessionDir = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/session";
        AJ::io::file_streamer::FileStreamer streamer(queue, pool, stopFlag,
            AJ::FileStreamingTypes::recording, sessionDir);

        AJ::AudioWriteInfo writeInfo;
        writeInfo.channels = channels;
        writeInfo.samplerate = 44100;
        bool ok = streamer.setWriteInfo(writeInfo, handler);
        assert(ok);

        // Thread pool
        AJ::utils::ThreadPool tp(2);

        size_t produced{0};

        // Streamer thread (consumer)
        tp.enqueue([&] {
            streamer.write(handler);
        });

        // Producer task
        tp.enqueue([&] {
            while (!stopFlag->load(std::memory_order_relaxed)) {
                std::this_thread::sleep_for(producer_sleep);

                AJ::utils::Buffer* buf = nullptr;
                while (!buf && !stopFlag->load(std::memory_order_relaxed)) {
                    buf = pool->pop(handler);
                }

                if(!buf){
                    break;
                }

                buf->frames = buffer_frames;

                // Generate a sine wave
                static double phase = 0.0;
                const double freq = 440.0;                     // A4 tone
                const double sampleRate = 44100.0;
                const double twoPi = 2.0 * M_PI;
                const double phaseIncrement = twoPi * freq / sampleRate;

                for (size_t i = 0; i < frame_samples; ++i) {
                    buf->data[i] = static_cast<float>(std::sin(phase));
                    phase += phaseIncrement;
                    if (phase >= twoPi) phase -= twoPi;       // wrap around
                }

                bool pushed = false;

                while (!pushed) {
                    pushed = queue->push(buf);
                }

                ++produced;
            }
        });

        // Let run
        std::this_thread::sleep_for(test_duration);
        stopFlag->store(true, std::memory_order_release);

        // wait to finish.
        while (queue->currentSize() > 0){
            continue;
        }
        
        std::cout << "  ✓ Multi-threaded streamer test validated.\n";
    }
};
