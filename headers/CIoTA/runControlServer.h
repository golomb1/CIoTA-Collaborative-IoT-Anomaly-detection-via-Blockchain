//
// Created by master on 18/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_RUN_CONTROL_SERVER_H
#define CIOTA_COMPLETE_PROJECT_RUN_CONTROL_SERVER_H

#include <headers/CppUtils/ThreadBarrier.h>
#include "runAgent.h"
#include "core/ModelUtilities.h"
#include "ObserverUtilities.h"
#include "../AnomalyDetection/ExtendedAnomalyControlServer.h"
#include "../CppUtils/Utils.h"
#include "core/ObserverCIoTABlockchainApplication.h"

namespace CIoTA {

    typedef struct ServerParameterPack {
        std::string &CIoTAGroup;
        unsigned short CIoTAPort;
        unsigned short controlPort;
        std::map<int, std::string> policies;
        size_t alpha;
        size_t completeBlockSize;
        size_t capacity;
        unsigned int fragmentSize;
        unsigned int receiveInterval;
    } ServerParameterPack;


    template<typename MODEL = unsigned char, unsigned short H>
    void startControlServer(CIoTA::ServerParameterPack *pack,
                            CryptoLibrary<H> *cryptoLibrary,
                            ObserverListener<MODEL> *listener,
                            CompressionLibrary *compressionLibrary,
                            ExtendedAnomalyControlServerListener* control,
                            BlockchainListener* blockchainListener,
                            CIoTA::ModelUtilities<MODEL> *utilities,
                            ThreadBarrier* stopFlag)
    {
        MulticastServerDescriptor server(pack->CIoTAGroup, pack->CIoTAPort);
        ExtendedAnomalyControlServer controlServer(control, pack->controlPort);
        for (auto &policy : pack->policies) {
            controlServer.addPolicy(policy.first, policy.second);
        }
        // blockchain listener
        // model utilities.
        ObserverUtilities<MODEL> viewerUtilities(utilities, listener);
        CIoTA::ObserverCIoTABlockchainApplication<MODEL, H> app(
                cryptoLibrary->getIdentifier(),
                pack->alpha,
                pack->completeBlockSize,
                blockchainListener,
                cryptoLibrary,
                &viewerUtilities);

        CIoTA::BaseAgent<MODEL, H> agent(
                pack->capacity,
                pack->fragmentSize,
                compressionLibrary,
                &server,
                pack->receiveInterval,
                &app);
        controlServer.run();
        agent.run();

        stopFlag->waitOn();

        agent.stop();
        controlServer.stop();
    }
}

#endif //CIOTA_COMPLETE_PROJECT_RUN_CONTROL_SERVER_H
