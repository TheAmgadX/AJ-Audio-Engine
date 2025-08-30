#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <cassert>
#include <condition_variable>

#include "core/buffer_pool.h"
#include "core/error_handler.h"
#include "core/thread_pool.h"

class BufferPoolTests {
public:
    static void run_all() {
        std::cout << "\nRunning BufferPool Tests\n";
        std::cout << "---------------------------------------------\n";

        // test_basic_allocation();
        // test_push_pop_single_thread();
        // test_invalid_push_pop();
        test_push_pop_multi_thread();

        std::cout << "All BufferPool Tests Completed Successfully.\n";
    }

private:
    static void test_basic_allocation() {
        std::cout << "\nTest: Basic Allocation\n";
        AJ::error::ConsoleErrorHandler handler;
        AJ::utils::BufferPool pool(handler, 16, 128, 2);

        assert(pool.isValid());
        assert(pool.capacity() == 16);
        assert(pool.channels() == 2);
        assert(pool.bufferSize() == 128 * 2); // stereo doubles samples

        std::cout << "  ✓ Allocation and metadata validated.\n";

        std::cout << "\nTest: Large Pool Allocation\n";

        auto start = std::chrono::high_resolution_clock::now();

        AJ::utils::BufferPool pool2(handler, 1024, 1024, 2);

        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> alloc_time = end - start;
        std::cout << "allocation time: " << alloc_time.count() << "\n";

        assert(pool2.isValid());
        assert(pool2.capacity() == 1024);
        assert(pool2.channels() == 2);
        assert(pool2.bufferSize() == 1024 * 2); // stereo doubles samples

        std::cout << "  ✓ Allocation Large Pool and metadata validated.\n";
    }

    static void test_push_pop_single_thread() {
        std::cout << "\nTest: Push/Pop Single Thread\n";
        AJ::error::ConsoleErrorHandler handler;
        AJ::utils::BufferPool pool(handler, 4, 16, 1);
        assert(pool.isValid());

        // Pop all buffers
        std::chrono::duration<double> pop_time;
        std::vector<float*> buffers;
        for (int i = 0; i < 4; i++) {
            auto start = std::chrono::high_resolution_clock::now();

            auto buf = pool.pop(handler);

            auto end = std::chrono::high_resolution_clock::now();
            pop_time = end - start;

            assert(buf != nullptr);
            buffers.push_back(buf);
        }
        
        std::cout << "Buffer poping time: " << pop_time.count() << "\n";
        // Now queue should be empty
        auto should_be_null = pool.pop(handler);
        assert(should_be_null == nullptr);

        // Push all buffers back
        std::chrono::duration<double> push_time;
        for (auto* b : buffers) {
            auto start = std::chrono::high_resolution_clock::now();

            bool success = pool.push(b, handler);

            auto end = std::chrono::high_resolution_clock::now();
            push_time = end - start;

            assert(success);
        }
        std::cout << "Buffer pushing time: " << push_time.count() << "\n";

        std::cout << "  ✓ Single-thread push/pop validated.\n";
    }

    static void test_push_pop_multi_thread() {
        std::cout << "\nTest: Push/Pop Multi-threaded\n";
        std::cout << "\nRunning BufferPool Integration Test (producer -> queue -> consumer)\n";
        std::cout << "------------------------------------------------------------------\n";

        AJ::error::ConsoleErrorHandler handler;

        // Test parameters
        const size_t queue_capacity = 1000;   // number of buffers in queue
        const size_t buffer_frames  = 1024;   // frames per buffer
        const uint8_t channels      = 1;      // mono
        const size_t frame_samples  = buffer_frames * channels;
        const std::chrono::seconds test_duration(5);
        const std::chrono::milliseconds consumer_sleep(2);
        const std::chrono::milliseconds producer_sleep(20);

        // Create the free buffer pool (pre-allocated)
        AJ::utils::BufferPool pool(handler, queue_capacity,
                        buffer_frames,
                        channels);

        assert(pool.isValid());
        std::cout << "BufferPool created: capacity=" << pool.capacity()
                  << " bufferFrames=" << pool.bufferSize() / channels
                  << " channels=" << pool.channels() << "\n";

        // Create a separate queue to carry filled buffers from producer -> consumer.
        // This queue starts empty (producer pushes filled buffers into it).
        AJ::utils::Queue filledQueue(/*empty=*/true,
                          /*queue_size=*/queue_capacity,
                          /*buffer_size=*/buffer_frames,
                          /*channels=*/channels,
                          handler);

        assert(filledQueue.isValid());
        std::cout << "Filled queue created: capacity=" << filledQueue.queueSize()
                  << " bufferFrames=" << filledQueue.bufferFrameCapacity() << "\n";

        // Pre-allocate a consumer-owned buffer (allocated outside threads to avoid per-iteration allocation)
        std::vector<float> consumerLocal(buffer_frames * channels, 0.0f);

        // Counters
        size_t produced{0};
        size_t consumed{0};
        std::atomic<bool> stopFlag{false};

        // Thread pool (use 2 threads, or more if desired)
        AJ::utils::ThreadPool tp(4);

        // Producer task: pop free buffer from pool, fill with 1.0f, push to filledQueue.
        tp.enqueue([&] {
            while (!stopFlag.load(std::memory_order_relaxed)) {
                std::this_thread::sleep_for(producer_sleep); // simulate waiting for port audio callback.

                // Obtain free buffer from pool (busy-wait)
                float* buf = nullptr;
                while (!stopFlag.load(std::memory_order_relaxed) && !buf) {
                    buf = pool.pop(handler);
                }

                if (stopFlag.load(std::memory_order_relaxed)) {
                    // If stopping and we didn't get a buffer, exit loop
                    break;
                }

                // Fill buffer with 1.0f
                for (size_t i = 0; i < frame_samples; ++i) {
                    buf[i] = 1.0f;
                }

                // Push the filled buffer to the filledQueue (busy-wait until success)
                bool pushed = false;
                while (!stopFlag.load(std::memory_order_relaxed) && !(pushed)) {
                    pushed = filledQueue.push(buf);
                }

                ++produced;
            } // while
        });

        // Consumer task: pop filled buffer from filledQueue, copy to consumerLocal, return buffer to pool, sleep 0.5s
        tp.enqueue([&] {
            while (!stopFlag.load(std::memory_order_relaxed)) {
                // Pop filled buffer (busy-wait)
                float* buf = nullptr;
        
                while (!buf && !stopFlag.load(std::memory_order_relaxed)) {
                    buf = filledQueue.pop();
                }
                
                if (stopFlag.load(std::memory_order_relaxed)) {
                    break;
                }

                // Simulate consumer processing time (0.5s)
                std::this_thread::sleep_for(consumer_sleep);
                
                bool ret = pool.push(buf, handler);

                if (!ret) {
                    // Should not happen: pool should accept returned buffers; report via handler and break
                    const std::string msg = "BufferPool push failed while returning buffer";
                    handler.onError(AJ::error::Error::BufferOverflow, msg);
                    break;
                }
                ++consumed;
            }
        }); 

        // Run for test_duration, then stop
        std::this_thread::sleep_for(test_duration);
        stopFlag.store(true, std::memory_order_relaxed);
        
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        size_t size = filledQueue.currentSize();
        std::cout << "Queue size: " << size << "\n";     

        while(true){
            float* buf = nullptr;
            
            buf = filledQueue.pop();
            
            if(!buf){
                break;
            }

            // Copy data from buf into consumer-local buffer
            // (this simulates consuming the data without returning pointer ownership)
            std::memcpy(consumerLocal.data(), buf, frame_samples * sizeof(float));

            // Return buffer to free pool
            bool ret = pool.push(buf, handler);
            if (!ret) {
                // Should not happen: pool should accept returned buffers; report via handler and break
                const std::string msg = "BufferPool push failed while returning buffer";
                handler.onError(AJ::error::Error::BufferOverflow, msg);
                break;
            }

            ++consumed;

            // Simulate consumer processing time (0.5s)
            std::this_thread::sleep_for(consumer_sleep);
        }

        std::cout << "Produced: " << produced << ", Consumed: " << consumed << "\n";

        // Basic assertions: at least some production and consumption occurred
        assert(produced > 0);
        assert(consumed > 0);
        // produced should be >= consumed (producer may be faster); allow inequality
        assert(produced == consumed);

        std::cout << "  ✓ BufferPool multi-threaded integration validated for " << test_duration.count() << "s\n";
        std::cout << "------------------------------------------------------------------\n";
    }

    static void test_invalid_push_pop() {
        std::cout << "\nTest: Invalid Push/Pop Cases\n";
        AJ::error::ConsoleErrorHandler handler;
        AJ::utils::BufferPool pool(handler, 2, 8, 1);
        assert(pool.isValid());

        // Null buffer push
        bool pushed = pool.push(nullptr, handler);
        assert(!pushed);

        // Empty queue pop
        float* b1 = pool.pop(handler);
        float* b2 = pool.pop(handler);
        assert(b1 && b2);
        float* b3 = pool.pop(handler);
        assert(b3 == nullptr);

        // Overflow push
        assert(pool.push(b1, handler));
        assert(pool.push(b2, handler));
        // This should fail (already full)
        bool overflow = pool.push(b2, handler);
        assert(!overflow);

        std::cout << "  ✓ Invalid cases handled correctly.\n";
    }
};
