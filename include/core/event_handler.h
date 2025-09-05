#pragma once 
#include "core/types.h"
#include "core/error_handler.h"
#include "core/thread_pool.h"

#include <atomic>
#include <memory>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>

namespace AJ::utils {

/**
 * @class IEventHandler
 * @brief Interface for handling recording or playback events.
 *
 * Implementations of this interface define how to process events
 * during an audio session (e.g., showing a UI, handling input, etc.).
 *
 * @note The provided thread pool guarantees at least one worker thread 
 *       is always available for tasks enqueued inside this function. 
 *       Event handlers can safely offload blocking or background work 
 *       (such as waiting for user input) without risking starvation of 
 *       the pool.
 * 
 *       See the specific functionality documentation (e.g., Recorder) to know 
 *       exactly how many available threads are guaranteed for this task. You can also use 
 *       ThreadPool::available() to check the number of currently available threads.

 */
class IEventHandler {
public:
    /**
     * @brief Called repeatedly during an audio session to process events.
     *
     * @param handler   Reference to an error handler for reporting issues.
     * @param threadPool Shared thread pool for managing background tasks.
     *                   Guaranteed to have at least one thread available.
     * @param pStopFllag Shared flag to signal stopping of the current process.
     */
    virtual void onProcess(
        AJ::error::IErrorHandler& handler,
        std::shared_ptr<AJ::utils::ThreadPool> threadPool,
        LFControlFlagPtr pStopFllag
    ) = 0;
};

/**
 * @class ConsoleRecordHandler
 * @brief Console-based implementation of IEventHandler for recording sessions.
 *
 * This handler:
 * - Spawns a background thread (via the thread pool) to wait for user input
 *   (Enter key) to stop recording.
 * - Displays a live timer (HH:MM:SS) in the console while recording is active.
 *
 * @note The thread pool provided to this handler is guaranteed to have
 *       at least one free thread, so offloading the blocking input task is always safe.
 */
class ConsoleRecordHandler : public IEventHandler {
public:
    /**
     * @brief Runs the console-based recording event handler.
     *
     * Displays a timer while recording and listens for user input to stop.
     *
     * @param handler   Reference to the error handler.
     * @param threadPool Shared thread pool used to run the input thread.
     *                   Guaranteed to have one available thread for recording.
     * 
     * @param pStopFlag Shared flag that signals when recording should stop.
     */
    void onProcess(AJ::error::IErrorHandler& handler, std::shared_ptr<ThreadPool> threadPool, LFControlFlagPtr pStopFlag) override {
        std::cout << "Recording... (press Enter to stop)\n";

        // ---- Input thread ----
        threadPool->enqueue([&]() {
            std::cin.get(); ///< Wait for Enter key
            pStopFlag->flag.store(true, std::memory_order_release);
        });

        using namespace std::chrono;
        auto start = steady_clock::now();

        // ---- Timer loop ----
        while (!pStopFlag->flag.load(std::memory_order_acquire)) {
            auto now = steady_clock::now();
            auto recorded = duration_cast<seconds>(now - start).count();

            int hours   = recorded / 3600;
            int minutes = (recorded % 3600) / 60;
            int seconds = recorded % 60;

            std::cout << "\rRecorded: "
                      << std::setw(2) << std::setfill('0') << hours   << ":"
                      << std::setw(2) << std::setfill('0') << minutes << ":"
                      << std::setw(2) << std::setfill('0') << seconds
                      << std::flush;

            std::this_thread::sleep_for(1s);
        }

        std::cout << "\nStopped.\n";
    }
};

} // namespace AJ::utils
