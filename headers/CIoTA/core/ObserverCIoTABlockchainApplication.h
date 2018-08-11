//
// Created by tomer on 21/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_OBSERVER_CIOTA_BLOCKCHAIN_APPLICATION_H
#define CIOTA_COMPLETE_PROJECT_OBSERVER_CIOTA_BLOCKCHAIN_APPLICATION_H

#include "BaseCIoTABlockchainApplication.h"

namespace CIoTA {
    template<typename MODEL, unsigned short H>
    class ObserverCIoTABlockchainApplication : public BaseCIoTABlockchainApplication<MODEL, H> {

    public:
        ObserverCIoTABlockchainApplication(
                const MemBuffer *from,
                size_t alpha,
                size_t blockSize,
                BlockchainListener *listener,
                CryptoLibrary<H> *cryptoLibrary,
                ModelUtilities<MODEL> *utilities)
                :
                BaseCIoTABlockchainApplication<MODEL,H>(from,
                                                       alpha,
                                                       blockSize,
                                                       listener,
                                                       cryptoLibrary,
                                                       utilities)
        {}


        void getSeed(size_t blockId, time_t timestamp, const char *prevBlockHash, MemBuffer *out) override {
            throw InvalidStateException("ObserverUtilities::isTesting should not be called");
        }

        void makeRecord(const char *block, size_t len, MemBuffer *out) override {
            throw InvalidStateException("ObserverUtilities::isTesting should not be called");
        }

        BlockchainApplication<H> *copy(MemBuffer *id) override {
            throw InvalidStateException("ObserverUtilities::isTesting should not be called");
        }

    };
}
#endif //CIOTA_COMPLETE_PROJECT_OBSERVER_CIOTA_BLOCKCHAIN_APPLICATION_H
