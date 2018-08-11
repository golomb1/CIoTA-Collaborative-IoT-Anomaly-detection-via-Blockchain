//
// Created by master on 24/01/18.
//

#include "../../headers/CppUtils/ThreadBarrier.h"

void ThreadBarrier::waitOn(){
    std::unique_lock<std::mutex> lck(mtx);
    while (!flag) {
        cv.wait(lck);
    }
}

void ThreadBarrier::notify() {
    std::unique_lock<std::mutex> lck(mtx);
    flag= true;
    cv.notify_all();
}

ThreadBarrier::ThreadBarrier() : flag(false) {

}

