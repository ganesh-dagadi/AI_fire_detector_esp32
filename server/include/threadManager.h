#ifndef THREAD_MANAGER
#define THREAD_MANAGER

#include <thread>
#include <vector>
#include <functional>
#include <mutex>

class ThreadManager {
private:
    std::vector<std::thread> threads;

    ThreadManager() {}

    ThreadManager(const ThreadManager&) = delete;
    ThreadManager& operator=(const ThreadManager&) = delete;

public:

    static ThreadManager& getInstance();

    void postRunnable(std::function<void()> Runnable);

    void waitForAllThreads();

    ~ThreadManager() {
        waitForAllThreads();
    }
};

#endif
