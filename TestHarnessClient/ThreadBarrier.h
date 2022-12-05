#ifndef __THREAD_BARRIER_H
#define __THREAD_BARRIER_H

#include <mutex>

class ThreadBarrier {

private:
    std::mutex threadMutex;
    std::condition_variable threadCv;
    int counter = 0;
    int waiting = 0;
    int threadCount;

public:
    ThreadBarrier(const ThreadBarrier&) = delete;
    ThreadBarrier& operator=(const ThreadBarrier&) = delete;
    explicit ThreadBarrier(int totalThreadCount): threadCount(totalThreadCount){}

    void wait()
    {
        std::unique_lock<std::mutex> lock(threadMutex);
        ++counter;
        ++waiting;
        threadCv.wait(lock, [&] { return counter >= threadCount; });
        threadCv.notify_one();
        --waiting;
        if (waiting == 0) { counter = 0; }
        lock.unlock();
    }
};

#endif __THREAD_BARRIER_H
