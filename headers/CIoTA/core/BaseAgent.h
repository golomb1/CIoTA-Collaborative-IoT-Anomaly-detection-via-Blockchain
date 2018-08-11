//
// Created by master on 18/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_BASE_AGENT_H
#define CIOTA_COMPLETE_PROJECT_BASE_AGENT_H


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


namespace CIoTA {

    /**
     * This is a base implementation of CIoTA Agent, which contain the logic shared by Agent and Viewer.
     * Support only viewing and without attempting to change or affect the system.
     * @tparam MODEL is the model type.
     * @tparam H is the size in byte of the HASH function used.
     */
    template<typename MODEL, unsigned short H>
    class BaseAgent {
    protected:
        BaseCIoTABlockchainApplication <MODEL, H> *_application;
        std::atomic<bool> _shouldStopFlag;
        std::condition_variable _cv;
        const size_t _maxTries;
        const unsigned int _receiveInterval;
        Blockchain<H> _chain;
        MessageFragmentationServer<BaseAgent<MODEL, H>> _server;
        std::vector<std::thread *> _workers;


    public:
        /**
         * Constructor for this abstract class.
         * @param capacity is the memory capacity to store the blockchain.
         * @param fragmentSize is the size of packets to sent during broadcast.
         * @param library is a compression library that allows for smaller broadcast fragments.
         * @param server is used to receiving and sending broadcasts.
         * @param receiveInterval is the inverval between udp receiving packets.
         * @param application is a CIoTA Application instance that handle the chain.
         */
        explicit BaseAgent(size_t capacity,
                           unsigned int fragmentSize,
                           CompressionLibrary* library,
                           BroadcastServer* server,
                           unsigned int receiveInterval,
                           BaseCIoTABlockchainApplication<MODEL, H>* application) :
                _application(application), _shouldStopFlag(false), _maxTries(fragmentSize * capacity),
                _receiveInterval(receiveInterval), _chain(capacity, _application),
                _server(fragmentSize, library, server)
        {
            _server.setMetadata(this);
            _server.addHandler(MESSAGE_CHAIN_TYPE, BaseAgent<MODEL, H>::receiveChainBroadcast);
        }

        /**
         * @return a pointer to the agent's chain.
         * @attention do not delete this pointer!.
         */
        Blockchain<H>* getBlockchain(){
            return &_chain;
        }

        void receiveIntelligence(){
            LOG_TRACE_4("await for message")
            bool found = false;
            if (_server.awaitForMessage()) {
		auto maxTries = (int) _maxTries;
                while (!(found = _server.receiveFragmentation()) && maxTries > 0) {
                    maxTries--;
                }
                LOG_TRACE_4("receive intelligence tries: {0:d} {1:d}", maxTries, found)
                _server.cleanFragment();
            }
        }

        /**
         * Stop the execution of this agent (all of it's threads)
         * If the agent wasn't running, nothing occurs.
         */
        void stop(){
            LOG_TRACE_4("CIoTA Agent start stopping")
            _shouldStopFlag = true;
            _cv.notify_all();
            for (auto &_worker : _workers) {
                if(_worker != nullptr) {
                    _worker->join();
                    delete _worker;
                    _worker = nullptr;
                }
                _workers.clear();
            }
            LOG_TRACE_4("CIoTA agent finish stopping")
            _shouldStopFlag = false;
        }


        /**
         * Start executing this agent on a seperate threads and returns.
         */
        virtual void run() {
            LOG_TRACE_4("CIoTA Agent start Running")
            _workers.push_back(
                    executingLoopingThread<BaseAgent<MODEL, H> *>(this, _receiveInterval, &_cv, &_shouldStopFlag,
                                                                  &BaseAgent<MODEL, H>::receiveIntelligence));
        }

    protected:

        /**
         * Receive a new broadcast for the blockchain.
         * @param agent is the instance of the agent that receive the broadcast.
         * @param buffer is broadcast content.
         * @param len is the buffer length.
         * @return true if the broadcasted chain was accepted.
         */
        static bool receiveChainBroadcast(BaseAgent<MODEL, H> *agent, const char *buffer, size_t len) {
            LOG_TRACE_4("receive broadcast - len was : {0:d}", len)
            agent->_application->startTransaction();
            SerializableBoundedQueue queue(buffer, len);
            bool res = agent->_chain.receiveChain(&queue) > 0;
            LOG_TRACE_4("receive broadcast - res was : {0}", res)
            agent->_application->endTransaction();
            return res >= 0;
        }
    };

}

#endif //CIOTA_COMPLETE_PROJECT_BASE_AGENT_H
