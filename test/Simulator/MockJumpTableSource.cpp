//
// Created by master on 18/01/18.
//

#include "MockJumpTableSource.h"
#include "../TEST.h"


bool attackFlag = false;
bool shuffleFlag = false;
size_t globalNumberOfDevices = 0;


bool simulateSource(LockedPointer<JumpTracer> *model, std::atomic<bool> *shouldStopFlag, pid_t pid) {
    uint64_t start = 0x00010000;
    uint64_t end = 0x0001a000;
    uint64_t range = end - start;
    uint64_t delta = (range / (2 * (globalNumberOfDevices + 1)));
    auto *addresses = new uint64_t[1 + globalNumberOfDevices];
    for (int i = 0; i < globalNumberOfDevices; i++) {
        addresses[i] = start + delta * i;
    }
    addresses[globalNumberOfDevices] = start + (delta * (pid + globalNumberOfDevices + 1));

    bool shuffled = false;
    bool attacked = false;
    size_t i = 0;

    while (!(*shouldStopFlag)) {
        uint64_t pc = addresses[i];
        model->use(&JumpTracer::consume, pc);
        uint64_t grade = 0;
        model->use(&JumpTracer::grade, &grade);
        if (attacked) {
            //ASSERT(grade < 100, true);
        } else if (shuffled) {
            ASSERT((grade > 50) + 1, 2);
        } else {
            //ASSERT((grade > 80) + 2, 3);
        }
        if (shuffleFlag && !shuffled) {
            size_t sh = (pid + 1) % globalNumberOfDevices;
            addresses[globalNumberOfDevices] = start + (delta * (sh + globalNumberOfDevices + 1));
            shuffled = true;
        } else if (attackFlag && !attacked) {
            addresses[1] = start + (delta * (pid + globalNumberOfDevices + 1));
            attacked = true;
            i = 0;
        } else {
            i = (i + 1) % (globalNumberOfDevices + 1);
        }
    }
    delete[] addresses;
}
