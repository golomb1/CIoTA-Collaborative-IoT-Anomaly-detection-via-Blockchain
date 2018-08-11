//
// Created by master on 24/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_THREAD_BARRIER_H
#define CIOTA_COMPLETE_PROJECT_THREAD_BARRIER_H

#include <mutex>
#include <condition_variable>

class ThreadBarrier{
private:
    std::mutex mtx;
    std::condition_variable cv;
    bool flag;

public:
    ThreadBarrier();

    void waitOn();

    void notify();

};

#endif //CIOTA_COMPLETE_PROJECT_THREAD_BARRIER_H
