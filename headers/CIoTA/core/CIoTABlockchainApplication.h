//
// Created by master on 10/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_CIOTA_BLOCKCHAIN_APPLICATION_H
#define CIOTA_COMPLETE_PROJECT_CIOTA_BLOCKCHAIN_APPLICATION_H


#include "ModelUtilities.h"
#include "CIoTA_Definitions.h"
#include "../../Blockchain/BlockchainApplication.h"
#include "../../Blockchain/BlockchainListener.h"
#include "../../CppUtils/CryptoLibrary.h"
#include "../../CppUtils/Logger.h"
#include "BaseCIoTABlockchainApplication.h"

namespace CIoTA {

    /**
     * @tparam MODEL is the type of anomaly detection model used.
     * @tparam H is the output size (in bytes) of the hash function.
     */
    template<typename MODEL, unsigned short H>
    class CIoTABlockchainApplication : public BaseCIoTABlockchainApplication<MODEL, H> {
    public:

        /**
         * Constructor that creates a new instance of CIoTABlockchainApplication.
         * @param membership is a buffer that contains the application membership.
         * @param alpha is the threshold used to detect deviation of new partial blocks.
         * @param completeBlockSize is the number of record that needed for a block to consider complete.
         * @param listener listener that receives notification about the blockchain state.
         * @param cryptoLibrary use to encrypt and sign on records.
         * @param utilities is a bridge to manage the anomaly detection model.
         */
        CIoTABlockchainApplication(const MemBuffer *membership, size_t alpha, size_t completeBlockSize,
                                   BlockchainListener *listener,
                                   CryptoLibrary<H> *cryptoLibrary, ModelUtilities<MODEL> *utilities)
                : BaseCIoTABlockchainApplication<MODEL, H>(membership, alpha, completeBlockSize, listener, cryptoLibrary,
                                                           utilities) {}

        /**
         * Execute the anomaly detection model.
         * @param cv is a condition variable that can be used to wake threads about the status of shouldStop flag.
         * @param shouldStop is a flag that indicate whether the execution of the model need to halt.
         */
        virtual void executeModel(std::condition_variable *cv, std::atomic<bool> *shouldStop) {
            BaseCIoTABlockchainApplication<MODEL, H>::_utilities->execute(cv, shouldStop);
        }

        /**
         * Functionality used when creating a new block.
         * @param blockId is the new block id.
         * @param timestamp the new block's timestamp.
         * @param prevBlockHash the new block's previous hash.
         * @param out is the buffer that will store the seed.
         */
        void getSeed(size_t blockId, time_t timestamp, const char *prevBlockHash, MemBuffer *out) override {
            out->append(BlockchainApplication<H>::getFrom(), BlockchainApplication<H>::getFromSize());
            out->appendValue(0, 1);
            out->append(&blockId, sizeof(size_t));
            out->append(&timestamp, sizeof(time_t));
        }

        /**
         * create a new record for sharing through the blockchain.
         * @param block that will include the record.
         * @param len is te length of the block.
         * @param out is the buffer that will store the record.
         */
        void makeRecord(const char *block, size_t len, MemBuffer *out) override {
            MemBuffer model;
            BaseCIoTABlockchainApplication<MODEL, H>::_utilities->getModel(&model);
            MemBuffer content;
            size_t seedSize = BLOCK_SEED_SIZE(block, H);
            content.append(&seedSize, sizeof(seedSize));
            content.append(BLOCK_SEED(block, H), seedSize);
            content.append(model.data(), model.size());
            BlockRecord record(BlockchainApplication<H>::getFrom(), BlockchainApplication<H>::getFromSize(),
                               content.data(), content.size());
            MemBuffer signedRecord;
            if (BaseCIoTABlockchainApplication<MODEL, H>::_cryptoLibrary->seal(BlockchainApplication<H>::getFrom(),
                                                                               BlockchainApplication<H>::getFromSize(),
                                                                               record.data(), record.size(),
                                                                               &signedRecord)) {
                BlockRecordHeader header = {
                        BlockchainApplication<H>::getFromSize(),
                        signedRecord.size()
                };
                out->append(&header, sizeof(header));
                out->append(BlockchainApplication<H>::getFrom(), header.fromLength);
                out->append(signedRecord.data(), header.contentLength);
            } else {
                BlockRecordHeader header = {
                        BlockchainApplication<H>::getFromSize(),
                        record.size()
                };
                out->append(&header, sizeof(header));
                out->append(BlockchainApplication<H>::getFrom(), header.fromLength);
                out->append(record.data(), header.contentLength);
            }
        }


        /**
         * Utilities for testing purpose.
         * @param id is the new id that needed to be assign to the copy.
         * @return a deep copy of this application instance.
         */
        BlockchainApplication<H> *copy(MemBuffer *id) override {
            return new CIoTABlockchainApplication(id,
                                                  BaseCIoTABlockchainApplication<MODEL, H>::_alpha,
                                                  BaseCIoTABlockchainApplication<MODEL, H>::_blockSize,
                                                  BaseCIoTABlockchainApplication<MODEL, H>::_listener,
                                                  BaseCIoTABlockchainApplication<MODEL, H>::_cryptoLibrary,
                                                  BaseCIoTABlockchainApplication<MODEL, H>::_utilities);
        }

        /**
         * @return true if the anomaly detection model is ready to be shared in the network, or false otherwise.
         */
        bool hasModelForSharing() {
            return BaseCIoTABlockchainApplication<MODEL, H>::_utilities->isTesting();
        }
    };

}
#endif //CIOTA_COMPLETE_PROJECT_CIOTA_BLOCKCHAIN_APPLICATION_H
