//
// Created by golombt on 25/12/2017.
//

#ifndef TEST_SIMULATOR_H
#define TEST_SIMULATOR_H

#include "../../headers/JumpTracer/JumpTracer.h"
#include "../../headers/CIoTA/core/Agent.h"
#include "../../headers/CppUtils/SHA256.h"
#include "../Mocks/SimulatorFileDescriptorWrapper.h"
#include "../../headers/JumpTracer/JumpTracerUtilities.h"

typedef struct SimulatorPack {
    SimulatorFileDescriptorWrapper *server;
    JumpTracer *tracer;
    JumpTracerUtilities *utilities;
    MemBuffer *id;
    CIoTA::CIoTABlockchainApplication<JumpTracer, SHA256_OUTPUT_SIZE> *app;
    CIoTA::Agent<JumpTracer, SHA256_OUTPUT_SIZE> *agnet;
    MockCompressionLibrary* compress;
} SimulatorPack;


class Simulator{
public:
    void serialSimulation(size_t numberOfDevices);

    void asynchronousSimulation(size_t numberOfDevices);

private:
    //SimulatorPack* runInstance(int id);
};


#endif //TEST_SIMULATOR_H
