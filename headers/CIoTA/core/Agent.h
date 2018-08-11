//
// Created by golombt on 26/12/2017.
//

#ifndef CIOTA_AGENT_H
#define CIOTA_AGENT_H


#include <condition_variable>
#include "ModelUtilities.h"
#include "CIoTABlockchainApplication.h"
#include "../../CppUtils/Logger.h"
#include "../../CppUtils/CryptoLibrary.h"
#include "../../CppUtils/ThreadWrapper.h"
#include "../../Blockchain/BlockchainApplication.h"
#include "../../Blockchain/Blockchain.h"
#include "../../Blockchain/BlockchainListener.h"
#include "../../NetworkingUtils/MessageFragmentationServer.h"
#include "BaseAgent.h"


namespace CIoTA {

    /**
     * Implementation of CIoTA Agent by extending BaseAgent functionalities.
     * @tparam MODEL is the anomaly detection model used.
     * @tparam H is the output size (in bytes) of the hash function that used by the blockchain.
     */
    template<typename MODEL, unsigned short H>
    class Agent : public BaseAgent<MODEL, H> {
    private:
        CIoTABlockchainApplication<MODEL, H> *_application;
        const unsigned short _port{};
        const unsigned int _shareInterval{};

    public:

        /**
         * Constructor - build a new CIoTA agent.
         * @param port that used by the network (broadcast port).
         * @param capacity is the memory size for storing the blockchain.
         * @param fragmentSize is the max broadcast packet size.
         * @param library - compression library that allows for data compression.
         * @param server - utilities to send and receive broadcasts.
         * @param shareInterval the interval (in seconds) between share attempts.
         * @param receiveInterval the interval (in seconds) between receiving attempts.
         * @param application is instance of CIoTA Application.
         */
        explicit Agent(unsigned short port,
                       size_t capacity,
                       unsigned int fragmentSize,
                       CompressionLibrary *library,
                       BroadcastServer *server,
                       unsigned int shareInterval,
                       unsigned int receiveInterval,
                       CIoTABlockchainApplication<MODEL, H> *application) :
                BaseAgent<MODEL, H>(capacity, fragmentSize, library, server, receiveInterval, application),
                _port{port}, _application(application),
                _shareInterval{shareInterval}
        { }


        /**
         * Execute the Anomaly detection model via the CIoTA application.
         */
        virtual void gatherIntelligence() {
            std::condition_variable *pCv = &(BaseAgent<MODEL, H>::_cv);
            std::atomic<bool> *pFlag = &(BaseAgent<MODEL, H>::_shouldStopFlag);
            _application->executeModel(pCv, pFlag);
        }

        /**
         * Attempt to share the local model through the blockchain.
         * And then broadcast the chain.
         */
        void shareIntelligence() {
            bool res = _application->hasModelForSharing();
            if (res) {
                LOG_TRACE_4("share Intelligence executed")
                BaseAgent<MODEL, H>::_chain.updatePartialBlock();
                BaseAgent<MODEL, H>::_chain.consumeChain([this](SerializableBoundedQueue *chain) {
                    LOG_TRACE_5("broadcast chain")
                    this->_server.broadcastFragmentation(_port, MESSAGE_CHAIN_TYPE, chain->serialize(),
                                                                        chain->serializationSize());
                });
            }
        }


        /**
         * Extend BaseAgent run function to supports both sharing intelligence and model updating.
         */
        void run() {
            LOG_TRACE_4("CIoTA Agent start Running")

            std::condition_variable *pCv = &(BaseAgent<MODEL, H>::_cv);
            std::atomic<bool> *pFlag = &(BaseAgent<MODEL, H>::_shouldStopFlag);
            BaseAgent<MODEL, H>::run();
            BaseAgent<MODEL, H>::_workers.push_back(executingLoopingThread<Agent<MODEL, H> *>(this, _shareInterval,
                                                                                              pCv, pFlag,
                                                                                              &Agent<MODEL, H>::shareIntelligence));
            BaseAgent<MODEL, H>::_workers.push_back(new std::thread(&Agent<MODEL, H>::gatherIntelligence, this));
        }

    };

}
#endif //CIOTA_AGENT_H
