#pragma once
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

namespace AJ::utils {

class ThreadPool {
public:
    ThreadPool(size_t num_threads = std::thread::hardware_concurrency()) {
        mNumThreads = num_threads;
        mWorking = num_threads;

        // Creating worker threads
        for (size_t i = 0; i < num_threads; ++i) {
            mThreads.emplace_back([this] {
                while (true) {
                    std::function<void()> task; 

                    {
                        std::unique_lock<std::mutex> lock(mMux);

                        mWorking--;
                        
                        cv_.wait(lock, [this] {
                            return !mTasks.empty() || mStop;
                        });
                        

                        if (mStop && mTasks.empty()) {
                            return;
                        }

                        // Get the next task from the queue
                        task = move(mTasks.front());
                        mTasks.pop();
                        mWorking++;
                    }

                    task();
                }
            });
        }
    }

    // Destructor to stop the thread pool
    ~ThreadPool()
    {
        {
            // Lock the queue to update the stop flag safely
            std::unique_lock<std::mutex> lock(mMux);
            mStop = true;
        }

        // Notify all threads
        cv_.notify_all();

        // Joining all worker threads to ensure they have
        // completed their tasks
        for (auto& thread : mThreads) {
            thread.join();
        }
    }

    // Enqueue task for execution by the thread pool
    void enqueue(std::function<void()> task)
    {
        {
            std::unique_lock<std::mutex> lock(mMux);
            mTasks.emplace(std::move(task));
        }
        cv_.notify_one();
    }

    int available() noexcept {
        std::unique_lock<std::mutex> lock(mMux);
        return mNumThreads - mWorking;
    }

private:
    std::vector<std::thread> mThreads;

    std::queue<std::function<void()> > mTasks;

    std::mutex mMux;

    std::condition_variable cv_;

    bool mStop = false;

    int mWorking;
    int mNumThreads;
};
}