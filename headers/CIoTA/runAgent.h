//
// Created by master on 18/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_RUN_AGENT_H
#define CIOTA_COMPLETE_PROJECT_RUN_AGENT_H

//
// Created by master on 17/01/18.
//

#include "core/Agent.h"
#include "core/ModelUtilities.h"
#include <headers/AnomalyDetection/ExtendedAnomalyListener.h>
#include <headers/NetworkingUtils/MulticastServerDescriptor.h>
#include <headers/CppUtils/Utils.h>
#include <headers/CppUtils/ThreadBarrier.h>

namespace CIoTA {

    typedef struct AgentParameterPack {
        std::string &CIoTAGroup;
        unsigned short CIoTAPort;
        std::string &serverAddress;
        unsigned short serverPort;
        std::map<int, Policy> policies;

        size_t alpha;
        size_t completeBlockSize;
        size_t capacity;
        unsigned int fragmentSize;
        unsigned int shareInterval;
        unsigned int receiveInterval;
    } AgentParameterPack;


    template<typename MODEL, unsigned short H>
    void startAgent(CIoTA::AgentParameterPack *pack,
                    CIoTA::ModelUtilities<MODEL> *utilities,
                    BlockchainListener *blockchainListener,
                    CryptoLibrary<H> *cryptoLibrary,
                    CompressionLibrary *compressionLibrary,
                    ThreadBarrier* flag)
    {
        MulticastServerDescriptor server(pack->CIoTAGroup, pack->CIoTAPort);
        ExtendedAnomalyListener listener(pack->serverAddress, pack->serverPort);
        for (auto &policy : pack->policies) {
            listener.addPreventionPolicy(policy.first, policy.second);
        }
        utilities->setListener(&listener);
        CIoTA::CIoTABlockchainApplication<MODEL, H> app(
                listener.getMembership(),
                pack->alpha,
                pack->completeBlockSize,
                blockchainListener,
                cryptoLibrary,
                utilities);

        CIoTA::Agent<MODEL, H> agent(
                pack->CIoTAPort,
                pack->capacity,
                pack->fragmentSize,
                compressionLibrary,
                &server,
                pack->shareInterval,
                pack->receiveInterval,
                &app);

        agent.run();
        flag->waitOn();
        agent.stop();
    }


}


#endif //CIOTA_COMPLETE_PROJECT_RUN_AGENT_H
