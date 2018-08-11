//
// Created by master on 18/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_MOCK_JUMP_TABLE_SOURCE_H
#define CIOTA_COMPLETE_PROJECT_MOCK_JUMP_TABLE_SOURCE_H


#include "../../headers/JumpTracer/JumpTracer.h"
#include "../../headers/CppUtils/LockedPointer.h"

extern bool attackFlag;
extern bool shuffleFlag;
extern size_t globalNumberOfDevices;

bool simulateSource(LockedPointer<JumpTracer> *model, std::atomic<bool> *shouldStopFlag, pid_t pid);

#endif //CIOTA_COMPLETE_PROJECT_MOCK_JUMP_TABLE_SOURCE_H
