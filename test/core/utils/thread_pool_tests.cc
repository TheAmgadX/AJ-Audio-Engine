#include <cassert>
#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include "core/thread_pool.h"


class ThreadPoolTests {
public:
    static void run_all() {
        std::cout << "\nRunning ThreadPool Tests\n";
        std::cout << "---------------------------------------------\n";

        test_threadpool_basic();
        test_threadpool_parallelism();
        test_threadpool_available();
    
        std::cout << "All RingBuffer Tests Completed Successfully.\n";
    }

    static void test_threadpool_basic() {
        std::cout << "\nTest: ThreadPool Basic Functionality\n";
        std::cout << "------------------------------------------------------------------\n";

        using namespace std::chrono_literals;

        // ThreadPool with 4 workers
        AJ::utils::ThreadPool pool(4);

        std::atomic<int> counter{0};

        // Enqueue 10 tasks that increment counter
        for (int i = 0; i < 10; ++i) {
            pool.enqueue([&counter, i] {
                std::this_thread::sleep_for(10ms); // simulate work
                ++counter;
            });
        }

        // Give tasks time to complete
        std::this_thread::sleep_for(500ms);

        std::cout << "Counter after tasks: " << counter.load() << "\n";

        assert(counter == 10);
        std::cout << "  ✓ Basic task execution validated\n";
        std::cout << "------------------------------------------------------------------\n";
    }

    static void test_threadpool_parallelism() {
        std::cout << "\nTest: ThreadPool Parallelism\n";
        std::cout << "------------------------------------------------------------------\n";

        using namespace std::chrono_literals;

        AJ::utils::ThreadPool pool(4);

        std::atomic<int> active{0};
        std::atomic<int> maxActive{0};

        // Enqueue 20 tasks
        for (int i = 0; i < 20; ++i) {
            pool.enqueue([&] {
                active++;
                maxActive.store(std::max(maxActive.load(), active.load()));
                std::this_thread::sleep_for(50ms); // simulate work
                active--;
            });
        }

        // Wait long enough for all tasks
        std::this_thread::sleep_for(2s);

        std::cout << "Max active tasks observed: " << maxActive.load() << "\n";

        assert(maxActive > 1); // tasks actually ran in parallel
        std::cout << "  ✓ Parallel execution validated\n";
        std::cout << "------------------------------------------------------------------\n";
    }

    static void test_threadpool_available() {
        std::cout << "\nTest: ThreadPool Availability Reporting\n";
        std::cout << "------------------------------------------------------------------\n";

        using namespace std::chrono_literals;

        AJ::utils::ThreadPool pool(2);

        std::atomic<int> done{0};

        // Enqueue blocking tasks
        pool.enqueue([&] {
            std::this_thread::sleep_for(500ms);
            ++done;
        });
        pool.enqueue([&] {
            std::this_thread::sleep_for(500ms);
            ++done;
        });

        std::this_thread::sleep_for(50ms);

        int available1 = pool.available();
        std::cout << "Available threads (expected 0): " << available1 << "\n";
        assert(available1 == 0);

        // Wait for tasks to finish
        std::this_thread::sleep_for(1s);

        int available2 = pool.available();
        std::cout << "Available threads (expected 2): " << available2 << "\n";
        assert(available2 == 2);
        
        std::cout << "  ✓ Availability tracking validated\n";
        std::cout << "------------------------------------------------------------------\n";
    }

};

