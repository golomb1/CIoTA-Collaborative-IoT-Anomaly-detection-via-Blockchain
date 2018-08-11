//
// Created by master on 18/01/18.
//


#include <iostream>
#include <headers/CppUtils/ThreadBarrier.h>
#include "../headers/CIoTA/runAgent.h"
#include "../headers/JumpTracer/JumpTracer.h"
#include "../headers/JumpTracer/JumpTracerUtilities.h"
#include "../headers/CppUtils/AES_Wrapper.h"
#include "Simulator/MockJumpTableSource.h"


#define TEST_GROUP     "225.0.0.37"
#define TEST_PORT      12345
#define CONTROL_PORT   12346
#define NUM_OF_ARGS    3


void Policy1() {
    std::cout << "POLICY 1" << std::endl;
}

void Policy2() {
    std::cout << "POLICY 2" << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc != 1 + NUM_OF_ARGS) {
        std::string programName(argv[0]);
        unsigned long pos = programName.find_last_of('/');
        programName = programName.substr(pos + 1);
        std::cout << "Usage \"" << programName << ": [ID] [completeBlockSize] [ServerIp]\"." << std::endl;
        std::cout << "ID is identifier for this device, needs to be unique between machines that executes the mock."
                  << std::endl;
        std::cout << "completeBlockSize is the number of devices in the mock simulation." << std::endl;
        std::cout << "ServerIp is the address of  CIoTA C&C server (the IP of the machine that executes MockServer)."
                  << std::endl;
        exit(EXIT_FAILURE);
    }

    pid_t id = static_cast<pid_t>(std::stoi(argv[1]));
    size_t completeBlockSize = static_cast<size_t>(std::stoi(argv[2]));
    std::string serverIp(argv[3]);
    std::string group(TEST_GROUP);
    globalNumberOfDevices = completeBlockSize;

    auto mask = static_cast<uint64_t>((~0) ^ 0x000000FF);
    size_t windows = 1000;
    uint64_t start = 0x00010000;
    uint64_t end = 0x0001a000;
    uint32_t threshold = 50;
    size_t pConsensus = 0;

    CIoTA::AgentParameterPack pack{
            group,              //std::string & CIoTAGroup;
            TEST_PORT,          //unsigned short CIoTAPort;
            serverIp,           //std::string &serverAddress;
            CONTROL_PORT,       //unsigned short serverPort;
            {
                    {1, Policy1},
                    {2, Policy2}
            },                  //std::map<int, Policy> policies;
            20,                 //size_t alpha;
            completeBlockSize,  //size_t completeBlockSize;
            10 * 1024,            //size_t capacity;
            256,                //unsigned int fragmentSize;
            20,                  //unsigned int shareInterval;
            20,                  //unsigned int receiveInterval;
    };

    JumpTracer emptyModel(windows, mask, start, end, threshold);
    JumpTracerUtilities utilities(id, pConsensus, completeBlockSize, 5, &emptyModel, simulateSource);

    MemBuffer idBuf(std::to_string(std::stoi(argv[1])));
    AES_SEALER sealer(&idBuf);
    SHA256_FUNC hash{};
    CryptoLibrary<SHA256_OUTPUT_SIZE> cryptoLibrary(&hash, &sealer);
    MockCompressionLibrary compressionLibrary;

    ThreadBarrier flag;
    std::thread t(CIoTA::startAgent<JumpTracer, SHA256_OUTPUT_SIZE>,&pack, &utilities, nullptr, &cryptoLibrary, &compressionLibrary, &flag);
    waitForInput();
    flag.notify();
    t.join();
}