#include "threadManager.h"

ThreadManager& ThreadManager::getInstance() {
        static ThreadManager instance;
        return instance;
    }

void ThreadManager::postRunnable(std::function<void()> Runnable){
    std::thread newThread(Runnable);
    threads.push_back(std::move(newThread));
}

void ThreadManager::waitForAllThreads() {
        for (std::thread& th : threads) { 
            if (th.joinable()) {
                th.join();
            }
        }
    }