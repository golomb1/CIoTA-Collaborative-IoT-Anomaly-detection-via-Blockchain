//
// Created by golombt on 28/12/2017.
//

#ifndef CIOTA_COMPLETE_PROJECT_THREAD_WRAPPER_H
#define CIOTA_COMPLETE_PROJECT_THREAD_WRAPPER_H


#include <thread>
#include <random>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <iostream>
#include "ThreadWrapper.h"



/**
 * Execute a function periodically.
 * @tparam D is the data type that the function accept as parameter.
 * @param data is the parameter to pass to the function.
 * @param interval between the function executions (in seconds).
 * @param cv is a conditional variable that can be used to notify that the function should stop (wake it from its sleep).
 * @param shouldStopFlag is a flag that indicate whether the thread should stop.
 * @param operation is the function to execute after the delay.
 */
template <typename D>
void workerLoopingThread(D data,
                         size_t interval,
                         std::condition_variable* cv,
                         std::atomic_bool* shouldStopFlag,
                         std::function<void(D)> operation){
    std::mutex cv_m;
    std::unique_lock<std::mutex> lk(cv_m);
    std::random_device r;

    // Choose a random mean between 1 and 6
    std::default_random_engine e1(r());
    std::uniform_int_distribution<size_t> uniform_dist(1, interval);
    while(!(*shouldStopFlag)) {
        using std::chrono::seconds;
        if (!(cv->wait_for(lk, seconds(interval + uniform_dist(e1)), [shouldStopFlag]() { return (shouldStopFlag->load()); })))
        {
            // timeout
            operation(data);
        }
    }
}



/**
 * Delay the execution of a function.
 * @tparam D is the data type that the function accept as parameter.
 * @param data is the parameter to pass to the function.
 * @param interval is the delay before the function should start executing (in seconds).
 * @param cv is a conditional variable that can be used to notify that the function should stop (wake it from its sleep).
 * @param shouldStopFlag is a flag that indicate whether the thread should stop.
 * @param operation is the function to execute after the delay.
 */
template <typename D>
void workerDelayedThread(D data,
                         size_t interval,
                         std::condition_variable* cv,
                         std::atomic_bool* shouldStopFlag,
                         std::function<void(D)> operation){
    std::mutex cv_m;
    std::unique_lock<std::mutex> lk(cv_m);
    using std::chrono::seconds;
    if (!(cv->wait_for(lk, seconds(interval), [shouldStopFlag]() { return (shouldStopFlag->load()); })))
    {
        // timeout
        operation(data);
    }
}

/**
 * Execute a new thread that perform a function periodically.
 * @tparam D is the data type that the function accept as parameter.
 * @param data is the parameter to pass to the function.
 * @param interval between the function executions (in seconds).
 * @param cv is a conditional variable that can be used to notify that the thread should stop (wake it from its sleep).
 * @param shouldStopFlag is a flag that indicate whether the thread should stop.
 * @param operation is the function to execute periodically.
 * @return pointer to the thread.
 * @attention delete this pointer after use.
 */
template <typename D>
std::thread* executingLoopingThread(D data,
                                    size_t interval,
                                    std::condition_variable* cv,
                                    std::atomic_bool* shouldStopFlag,
                                    std::function<void(D)> operation)
{
    return new std::thread(workerLoopingThread<D>,
                           data,
                           interval,
                           cv,
                           shouldStopFlag,
                           operation);
}



#endif //CIOTA_COMPLETE_PROJECT_THREAD_WRAPPER_H
