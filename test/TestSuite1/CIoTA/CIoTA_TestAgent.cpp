//
// Created by golombt on 28/12/2017.
//

#include <iostream>
#include "CIoTA_TestAgent.h"
#include "../../../headers/CppUtils/MemBuffer.h"
#include "../../../headers/CIoTA/core/Agent.h"
#include "../../Mocks/BlockchainTestUtils.h"
#include "CIoTA_TestApplication.h"
#include "../../Mocks/SimulatorFileDescriptorWrapper.h"
//#include "../../Simulator/SimulatorFileDescriptorWrapper.h"


bool CIoTA_TestAgent(){
    std::cout << "CIoTA_TestAgent\n";
    EmptySHA256CryptoLibraryTest cryptoLibrary;
    StringModelUtilities utilities;
    MockCompressionLibrary library;
    CIoTA::Agent<std::string, SHA256_OUTPUT_SIZE>* agents[5];
    MemBuffer* ids[5];
    SimulatorFileDescriptorWrapper* servers[5];
    CIoTA::CIoTABlockchainApplication<std::string, SHA256_OUTPUT_SIZE>* apps[5];
    for(size_t i=0; i < 5; i++){
        std::string idStr = std::string("Id") + std::to_string(i+1);
        ids[i] = new MemBuffer(idStr.data(), idStr.size());
        servers[i] = new SimulatorFileDescriptorWrapper(i+1);
        apps[i] = new CIoTA::CIoTABlockchainApplication<std::string, SHA256_OUTPUT_SIZE>(ids[i], 50, 3, nullptr, &cryptoLibrary, &utilities);
        agents[i] = new CIoTA::Agent<std::string, SHA256_OUTPUT_SIZE>(9999, 1024*1024, 256, &library , servers[i], 30, 30, apps[i]);
    }
    agents[0]->shareIntelligence();
    for(int i=1; i < 5; i++) {
        agents[i]->receiveIntelligence();
        agents[i]->shareIntelligence();
    }
    for (auto &agent : agents) {
        agent->receiveIntelligence();
        agent->getBlockchain()->consumeChain([](SerializableBoundedQueue *c) {
            ASSERT(c->size(), 1);
        });
    }
    for(size_t i=0; i < 5; i++){
        delete agents[i];
        delete apps[i];
        delete servers[i];
        delete ids[i];
    }
}