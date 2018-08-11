//
// Created by golombt on 25/12/2017.
//

#include <random>
#include "Simulator.h"
#include "../TestSuite1/CIoTA/CIoTA_TestApplication.h"
#include "MockJumpTableSource.h"


void cleanPackets() {
    SimulatorFileDescriptorWrapper::packets.clear();
    for (auto p : SimulatorFileDescriptorWrapper::lastReceivedPacket) {
        SimulatorFileDescriptorWrapper::lastReceivedPacket[p.first] = 0;
    }
}

SimulatorPack *runInstance(size_t numOfDevices, unsigned long i, EmptySHA256CryptoLibraryTest *library) {
    auto pid = static_cast<pid_t>(i);
    auto mask = static_cast<uint64_t>((~0) ^ 0x000000FF);
    size_t windows = 1000;
    uint64_t start = 0x00010000;
    uint64_t end = 0x0001a000;
    uint32_t threshold = 50;

    size_t pConsensus = 0;

    auto *server = new SimulatorFileDescriptorWrapper(static_cast<size_t>(i + 1));
    auto* emptyModel = new JumpTracer(server, windows, mask, start, end, threshold);
    auto *utilities = new JumpTracerUtilities(pid, pConsensus, numOfDevices, 5, emptyModel, simulateSource);
    std::string from = std::string("Id") + std::to_string(i + 1);
    auto* fromBuffer = new MemBuffer(from.data(), from.size());
    fromBuffer->appendValue(0, 1);
    auto *app = new CIoTA::CIoTABlockchainApplication<JumpTracer, SHA256_OUTPUT_SIZE>(fromBuffer, 20, numOfDevices,
                                                                                      nullptr, library, utilities);
    auto* compress = new MockCompressionLibrary();
    auto* agent = new CIoTA::Agent<JumpTracer, SHA256_OUTPUT_SIZE>(9999, 512, 256, compress , server, 5, 5, app);
    agent->run();
    return new SimulatorPack{server, emptyModel, utilities, fromBuffer, app, agent, compress};
}


void Simulator::serialSimulation(size_t numberOfDevices) {
    using namespace std::chrono_literals;
    auto *library = new EmptySHA256CryptoLibraryTest();
    globalNumberOfDevices = numberOfDevices;
    SimulatorFileDescriptorWrapper::globalCounter = numberOfDevices;
    std::vector<SimulatorPack*> agents;

    std::cout << "\tStart agents\n";

    for (unsigned long i = 0; i < numberOfDevices; i++) {
        agents.push_back(runInstance(numberOfDevices, i, library));
        std::cout << "\t\tAgent " << i << " running\n";
        std::this_thread::sleep_for(20s);
        agents.at(i)->agnet->stop();
        std::cout << "\t\tAgent " << i << " stopped\n";
        // assert the models.
        const std::map<CodeAddress, std::map<CodeAddress, uint64_t>> &table =
                agents.at(i)->utilities->getTable();
        for (auto row : table) {
            ASSERT(row.second.size(), 1);
        }
        cleanPackets();
    }

    std::cout << "\tStart communication\n";

    agents.at(0)->agnet->shareIntelligence();
    for (int k = 1; k >= 0; k--) {
        for (auto i = static_cast<unsigned long>(k); i < numberOfDevices; i++) {
            agents.at(i)->agnet->run();
            std::cout << "\t\tAgent " << i << " running\n";
            std::this_thread::sleep_for(20s);
            agents.at(i)->agnet->stop();
            std::cout << "\t\tAgent " << i << " stopped\n";
            cleanPackets();
            agents.at(i)->agnet->shareIntelligence();
        }
    }

    for (unsigned long i = 0; i < numberOfDevices; i++) {
        const std::map<CodeAddress, std::map<CodeAddress, uint64_t>> &table =
                agents.at(i)->utilities->getTable();
        bool found = false;
        for (const auto &row : table) {
            found |= (row.second.size() == numberOfDevices);
        }
        ASSERT(found, true);
    }

    shuffleFlag = true;
    std::cout << "\tStart shuffle\n";

    for (unsigned long i = 0; i < numberOfDevices; i++) {
        agents.at(i)->agnet->run();
        std::cout << "\t\tAgent " << i << " running\n";
        std::this_thread::sleep_for(20s);
        agents.at(i)->agnet->stop();
        std::cout << "\t\tAgent " << i << " stopped\n";
        cleanPackets();
        agents.at(i)->agnet->shareIntelligence();
    }
    ASSERT(SimulatorFileDescriptorWrapper::globalCounter, numberOfDevices);

    attackFlag = true;
    std::cout << "\tStart attack\n";


    for (unsigned long i = 0; i < numberOfDevices; i++) {
        agents.at(i)->agnet->run();
        std::cout << "\t\tAgent " << i << " running\n";
        std::this_thread::sleep_for(20s);
        agents.at(i)->agnet->stop();
        std::cout << "\t\tAgent " << i << " stopped\n";
        cleanPackets();
        agents.at(i)->agnet->shareIntelligence();
    }
    ASSERT(SimulatorFileDescriptorWrapper::globalCounter, 0);

    std::cout << "\tFinish\n";
    for(auto *agent : agents){
        delete(agent->agnet);
        delete(agent->app);
        delete(agent->id);
        delete(agent->server);
        delete(agent->tracer);
        delete(agent->utilities);
        delete(agent->compress);
        delete(agent);
    }
    delete library;
}


void Simulator::asynchronousSimulation(size_t numberOfDevices) {
    using namespace std::chrono_literals;
    auto *library = new EmptySHA256CryptoLibraryTest();
    globalNumberOfDevices = numberOfDevices;
    SimulatorFileDescriptorWrapper::globalCounter = numberOfDevices;
    std::vector<SimulatorPack *> agents;

    std::cout << "\tStart agents\n";

    for (unsigned long i = 0; i < numberOfDevices; i++) {
        agents.push_back(runInstance(numberOfDevices, i, library));
        std::cout << "\t\tAgent " << i << " running\n";
    }
    for (int i = 0; i < 18; i++) {
        std::this_thread::sleep_for(20s);
        cleanPackets();
    }
    std::cout << "\tTest communication\n";

    for (unsigned long i = 0; i < numberOfDevices; i++) {
        const std::map<CodeAddress, std::map<CodeAddress, uint64_t>> &table =
                agents.at(i)->utilities->getTable();
        bool found = false;
        for (const auto &row : table) {
            found |= (row.second.size() == numberOfDevices);
        }
        ASSERT(found, true);
    }

    shuffleFlag = true;
    std::cout << "\tStart shuffle\n";
    std::this_thread::sleep_for(30s);
    ASSERT(SimulatorFileDescriptorWrapper::globalCounter, numberOfDevices);

    attackFlag = true;
    std::cout << "\tStart attack\n";
    std::this_thread::sleep_for(30s);
    ASSERT(SimulatorFileDescriptorWrapper::globalCounter, 0);

    std::cout << "\tFinish\n";
    for (auto *agent : agents) {
        agent->agnet->stop();
        delete (agent->agnet);
        delete (agent->app);
        delete (agent->id);
        delete (agent->server);
        delete (agent->tracer);
        delete (agent->utilities);
        delete (agent->compress);
        delete (agent);
    }
    delete library;
}


