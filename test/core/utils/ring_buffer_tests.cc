#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <cassert>
#include <condition_variable>

#include "core/ring_buffer.h"
#include "core/error_handler.h"
#include "core/thread_pool.h"

class RingBufferTests {
public:
    static void run_all() {
        std::cout << "\nRunning RingBuffer Tests\n";
        std::cout << "---------------------------------------------\n";

        test_basic_allocation();
        test_push_pop_single_thread();
        test_invalid_cases();
        test_push_pop_multi_thread();

        std::cout << "All RingBuffer Tests Completed Successfully.\n";
    }

private:
    static void test_basic_allocation() {
        std::cout << "\nTest: Basic Allocation\n";
        AJ::error::ConsoleErrorHandler handler;
        AJ::utils::RingBuffer rb(1000, 2, handler);

        assert(rb.isValid());
        assert(rb.frameCapacity() == 1024);
        assert(rb.channels() == 2);
        assert(rb.samplesCapacity() == 1024 * 2); // stereo doubles samples

        std::cout << "  ✓ Allocation and metadata validated.\n";
    }

    static void test_push_pop_single_thread() {
        std::cout << "\nTest: write/read Single Thread\n";
        AJ::error::ConsoleErrorHandler handler;
        AJ::utils::RingBuffer rb = AJ::utils::RingBuffer(1000, 2, handler);
        assert(rb.isValid());

        // push samples sample using writeFrame
        std::chrono::duration<double> push_time;

        float *buffer = new float[2];
        for(int i = 0; i < 1024; ++i){
            buffer[0] = 1.0f;
            buffer[1] = 1.0f;

            auto start = std::chrono::high_resolution_clock::now();
            
            bool valid = rb.writeFrame(buffer);

            auto end = std::chrono::high_resolution_clock::now();
            push_time = end - start;

            assert(valid);
        }
        std::cout << "Buffer write frame time: " << push_time.count() << "\n";


        // Pop all buffers
        std::chrono::duration<double> pop_time;
        float *main_buffer = new float[2048];
        int j = 0;
        for (int i = 0; i < 1024; i++) {
            auto start = std::chrono::high_resolution_clock::now();

            bool valid = rb.readFrame(buffer);

            auto end = std::chrono::high_resolution_clock::now();
            pop_time = end - start;

            assert(valid);

            main_buffer[j++] = buffer[0];
            main_buffer[j++] = buffer[1];
        }
        
        std::cout << "Buffer reading frame time: " << pop_time.count() << "\n";

        // Now queue should be empty
        bool should_be_false = rb.readFrame(buffer);
        assert(should_be_false == false);

        // Push all buffer's data back
        auto start = std::chrono::high_resolution_clock::now();

        size_t frames = rb.writeFrames(main_buffer, 1024);

        auto end = std::chrono::high_resolution_clock::now();
        push_time = end - start;

        std::cout << "Buffer write frames time: " << push_time.count() << "\n";

        assert(frames == 1024);
        
        // reading all data again.
        start = std::chrono::high_resolution_clock::now();

        frames = rb.readFrames(main_buffer, 1024);

        end = std::chrono::high_resolution_clock::now();
        pop_time = end - start;

        std::cout << "Buffer read frames time: " << pop_time.count() << "\n";
        
        assert(frames == 1024);


        std::cout << "  ✓ Single-thread write/read validated.\n";
    }
        
    static void test_push_pop_multi_thread() {
        std::cout << "\nTest: write/read Multi-threaded RingBuffer\n";
        std::cout << "------------------------------------------------------------------\n";

        AJ::error::ConsoleErrorHandler handler;

        const size_t buffer_frames  = 1024;
        const uint8_t channels      = 2;
        const std::chrono::seconds test_duration(5);
        const std::chrono::milliseconds producer_sleep(20);
        const std::chrono::milliseconds consumer_sleep(2);

        AJ::utils::RingBuffer rb(buffer_frames * 8, channels, handler);
        assert(rb.isValid());

        std::vector<float> consumerLocal(buffer_frames * channels, 0.0f);

        size_t produced{0};
        size_t consumed{0};
        std::atomic<bool> stopFlag{false};

        AJ::utils::ThreadPool pool(2);

        pool.enqueue([&] {
            std::vector<float> local(buffer_frames * channels, 1.0f);
            while (!stopFlag.load(std::memory_order_relaxed)) {
                std::this_thread::sleep_for(producer_sleep);
                size_t written = rb.writeFrames(local.data(), buffer_frames);
                if (written == buffer_frames) {
                    ++produced;
                }
            }
        });

        pool.enqueue([&] {
            while (!stopFlag.load(std::memory_order_relaxed)) {
                size_t read = rb.readFrames(consumerLocal.data(), buffer_frames);
                if (read == buffer_frames) {
                    ++consumed;
                    std::this_thread::sleep_for(consumer_sleep);
                }
            }
        });

        std::this_thread::sleep_for(test_duration);
        stopFlag.store(true, std::memory_order_relaxed);

        // Give consumer chance to drain remaining data
        size_t finalDrain = rb.readFrames(consumerLocal.data(), buffer_frames);
        if (finalDrain == buffer_frames) {
            ++consumed;
        }

        std::cout << "Produced blocks: " << produced << ", Consumed blocks: " << consumed << "\n";

        assert(produced > 0);
        assert(consumed > 0);
        assert(consumed == produced);

        std::cout << "  ✓ RingBuffer multi-threaded integration validated for "
                << test_duration.count() << "s\n";
        std::cout << "------------------------------------------------------------------\n";
    }

    static void test_invalid_cases() {
        std::cout << "\nTest: Invalid write/read Cases (RingBuffer)\n";
        AJ::error::ConsoleErrorHandler handler;
        AJ::utils::RingBuffer rb(128, 1, handler);
        assert(rb.isValid());

        float sample[1] = {0.5f};
        float out[1];

        // Null pointer write
        bool w = rb.writeFrame(nullptr);
        assert(!w);

        // Null pointer read
        bool r = rb.readFrame(nullptr);
        assert(!r);

        // Fill until full
        size_t frames_written = 0;
        while (rb.writeFrame(sample)) {
            ++frames_written;
        }
        assert(frames_written > 0);

        // Now write should fail (full)
        bool should_fail = rb.writeFrame(sample);
        assert(!should_fail);

        // Drain completely
        size_t frames_read = 0;
        while (rb.readFrame(out)) {
            ++frames_read;
        }
        assert(frames_read == frames_written);

        // Now read should fail (empty)
        bool should_fail_read = rb.readFrame(out);
        assert(!should_fail_read);

        std::cout << "  ✓ RingBuffer invalid cases handled correctly.\n";
    }

};
