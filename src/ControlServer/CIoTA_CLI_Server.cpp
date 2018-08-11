//
// Created by master on 23/01/18.
//

#include <iostream>
#include "../../headers/CppUtils/INIReader.h"
#include "../../headers/JumpTracer/JumpTracer.h"
#include "../../headers/JumpTracer/JumpTracerUtilities.h"
#include "../../headers/AnomalyDetection/ExtendedAnomalyControlServer.h"
#include "../../headers/CIoTA/runControlServer.h"
#include "../../headers/CppUtils/AES_Wrapper.h"
#include "../../headers/CIoTA/LogObserverListener.h"


#define NUM_OF_MANDATORY_ARGS    1
#define NUM_OF_OPT_ARGS          1
#define OPT_ARG_LOG              2





int main(int argc, char *argv[]) {
    if (!(argc >= NUM_OF_MANDATORY_ARGS + 1 && argc <= NUM_OF_MANDATORY_ARGS + NUM_OF_OPT_ARGS + 1)) {
        printUsage(argv[0]);
    }

    INIReader reader(argv[1]);
    if (reader.ParseError() < 0) {
        std::cout << "Can't load 'test.ini'\n";
        printUsage(argv[0]);
    }

    pid_t pid = static_cast<pid_t>(reader.GetInteger("Tracer", "pid", -1));
    CodeAddress mask =       reader.GetU64Integer("Tracer", "mask", std::numeric_limits<CodeAddress>::max());
    CodeAddress filterFrom = reader.GetU64Integer("Tracer", "filterFrom", 0);
    CodeAddress filterTo   = reader.GetU64Integer("Tracer", "filterTo", std::numeric_limits<CodeAddress>::max());
    size_t window    = reader.GetUInteger("Tracer", "windows", 10*1000);
    size_t threshold = reader.GetUInteger("Tracer", "threshold", 50);
    unsigned int trainInterval = static_cast<unsigned int>(reader.GetUInteger("Tracer", "trainInterval", 30 * 60));


    std::string group   = reader.Get("CIoTA", "multicastGroup", "");
    unsigned short port = static_cast<unsigned short>(reader.GetUInteger("CIoTA", "port", 0));
    std::string controlServerAddress = reader.Get("CIoTA", "controlIP", "");
    unsigned short controlServerPort = static_cast<unsigned short>(reader.GetUInteger("CIoTA", "controlPort", 0));

    size_t pConsensus    = reader.GetUInteger("CIoTA", "pConsensus", std::numeric_limits<size_t>::max());
    uint shareInterval   = static_cast<uint>(reader.GetUInteger("CIoTA", "shareInterval", std::numeric_limits<uint>::max()));
    uint receiveInterval = static_cast<uint>(reader.GetUInteger("CIoTA", "receiveInterval", std::numeric_limits<uint>::max()));
    size_t alpha             = reader.GetUInteger("CIoTA", "alpha", 2);
    size_t completeBlockSize = reader.GetUInteger("CIoTA", "completeBlockSize", std::numeric_limits<size_t>::max());

    size_t capacity     = reader.GetUInteger("CIoTA", "capacity", std::numeric_limits<size_t>::max());
    uint fragmentSize = static_cast<uint>(reader.GetUInteger("CIoTA", "fragmentSize", std::numeric_limits<uint>::max()));

    std::string id        = reader.Get("CIoTA", "EntityKey", "");

    size_t loggerSize     = reader.GetU64Integer("Log", "loggerSize", 1048576 * 5);
    size_t loggerRotation = reader.GetU64Integer("Log", "loggerRotation", 3);

    MemBuffer identificationBuffer(&id);

    CIoTA::ServerParameterPack pack{
            group,                   //std::string &CIoTAGroup;
            port,                    //unsigned short CIoTAPort;
            controlServerPort,       //unsigned short controlPort;
            {
                    {1, "Ignore"},
                    {2, "Kill the application"},
            },                      //std::map<int, std::string> policies;
            alpha,                  //size_t alpha;
            completeBlockSize,      //size_t completeBlockSize;
            capacity,               //size_t capacity;
            fragmentSize,           //unsigned int fragmentSize;
            receiveInterval         //unsigned int receiveInterval;
    };

    AES_SEALER sealer(&identificationBuffer);
    SHA256_FUNC hash{};
    CryptoLibrary<SHA256_OUTPUT_SIZE> cryptoLibrary(&hash, &sealer);
    LogObserverListener<JumpTracer>* listener = nullptr;
    if(argc > OPT_ARG_LOG){
        listener = new LogObserverListener<JumpTracer>(argv[OPT_ARG_LOG], loggerSize, loggerRotation);
    }
    MockCompressionLibrary compressionLibrary;
    CliExtendedAnomalyControlServerListener control;
    ThreadBarrier flag;
    std::thread t(CIoTA::startControlServer<JumpTracer, SHA256_OUTPUT_SIZE>,
                &pack, &cryptoLibrary, listener, &compressionLibrary,
                &control, nullptr, nullptr, &flag);
    flag.notify();
    t.join();
    delete listener;
}