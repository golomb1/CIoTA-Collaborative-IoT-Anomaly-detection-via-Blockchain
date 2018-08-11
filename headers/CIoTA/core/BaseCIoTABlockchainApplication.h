//
// Created by master on 18/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_BASE_CIOTA_BLOCKCHAIN_APPLICATION_H
#define CIOTA_COMPLETE_PROJECT_BASE_CIOTA_BLOCKCHAIN_APPLICATION_H

#include "../../Blockchain/BlockchainApplication.h"
#include "../../Blockchain/BlockchainListener.h"
#include "../../CppUtils/CryptoLibrary.h"
#include "ModelUtilities.h"
#include "CIoTA_Definitions.h"
#include "../../CppUtils/Logger.h"

namespace CIoTA {

    template<typename MODEL, unsigned short H>
    class BaseCIoTABlockchainApplication : public BlockchainApplication<H> {
    protected:
        size_t _alpha{};
        size_t _blockSize{};
        BlockchainListener *_listener{};
        CryptoLibrary<H> *_cryptoLibrary;
        ModelUtilities <MODEL> *_utilities;

    public:
        /**
         * Constructor for the CIoTA agent.
         * @param from is a buffer that hold the agent identification.
         * @param alpha is the threshold used when testing a new model.
         * @param blockSize is the the number of record needed to consider a block as complete.
         * @param listener is a blockchain listener to log on events.
         * @param cryptoLibrary is the cryptographic library to use.
         * @param utilities is the model utilities that manage the anomaly detection model.
         */
        BaseCIoTABlockchainApplication(const MemBuffer *from,
                                       size_t alpha,
                                       size_t blockSize,
                                       BlockchainListener *listener,
                                       CryptoLibrary<H> *cryptoLibrary,
                                       ModelUtilities <MODEL> *utilities) :
                BlockchainApplication<H>(from, true),
                _alpha(alpha),
                _blockSize(blockSize),
                _listener(listener),
                _cryptoLibrary(cryptoLibrary),
                _utilities(utilities) {
        }

        /**
         * Block is consider complete if he include exactly _blockSize records.
         * @param block to test
         * @param len the length of the block
         * @return true if the block is consider complete and false otherwise.
         */
        bool isCompletedBlock(const char *block, size_t) override {
            size_t size = BLOCK_NUM_OF_RECORDS(block, H);
            return size == _blockSize;
        }

        /**
         * Calculate an hash of a given block.
         * @param out is the output buffer that should include the hash value.
         * @param pBlock is the block to calculate its hash.
         * @param len is the length of the block.
         */
        void getHash(char out[H], const char *pBlock, size_t len) override {
            _cryptoLibrary->hash(out, pBlock, len);
        }

        /**
         * Check if a block is acceptable, if he valid and stand in CIoTA limitations.
         * 1. The block shouldn't include more than one record from the same participant.
         * 2. All record should use the same seed as the block seed + timestamp.
         * 3. Each record should be unsealed successfully.
         * 4. The block timestamp should be valid.
         * @param pBlock to test.
         * @param len of the block.
         * @return true if acceptable and false otherwise.
         */
        bool isBlockAcceptable(const char *pBlock, size_t len) override {
            if (BLOCK_NUM_OF_RECORDS(pBlock, H) > _blockSize) {
                return false;
            }
            // check that all record have the same seed.
            // check that there isn't two records from the same source.
            BlockRecordIterator it = blockRecordsBegin<H>(pBlock);
            BlockRecordIterator end = blockRecordsEnd<H>(pBlock);

            const char *seed = BLOCK_SEED(pBlock, H);
            size_t seedLength = BLOCK_SEED_SIZE(pBlock, H);
            // TODO check seed timestamp and discard if too old.
            for (; it != end; ++it) {
                BlockRecordIterator innerIt = it;
                const char *from = RECORD_FROM(*it);
                size_t fromLen = RECORD_FROM_LENGTH(*it);
                innerIt++;
                for (; innerIt != end; ++innerIt) {
                    if (RECORD_FROM_LENGTH(*innerIt) == fromLen
                        && memcmp(RECORD_FROM(*innerIt), from, RECORD_FROM_LENGTH(*innerIt)) == 0) {
                        return false;
                    }
                }

                const char *content = RECORD_CONTENT(*it);
                size_t contentSize = RECORD_CONTENT_LENGTH(*it);
                MemBuffer data;
                if (!_cryptoLibrary->unseal(from, fromLen, content, contentSize, &data)) {
                    return false;
                }
                size_t recordSeedLen = CIoTA_RECORD_SEED_LENGTH(data.data());
                const char *recordSeed = CIoTA_RECORD_SEED(data.data());
                if (seedLength != recordSeedLen ||
                    memcmp(seed, recordSeed, seedLength) != 0) {
                    return false;
                }
            }
            return true;
        }


        /**
         * Transfer the given block's records to a set of records.
         * @param pBlock is a pointer to the block to translate.
         * @param len of the block.
         * @param set is the output set.
         * @return true if success and false otherwise.
         */
        bool collectModels(const char *pBlock, size_t len, std::vector<MemBuffer *> &set) {
            // check that all record have the same seed.
            // check that there isn't two records from the same source.
            BlockRecordIterator it = blockRecordsBegin<H>(pBlock);
            BlockRecordIterator end = blockRecordsEnd<H>(pBlock);
            for (; it != end; ++it) {
                const char *from = RECORD_FROM(*it);
                LOG_TRACE("COLLECT MODEL FROM {0}", from)
                size_t fromLen = RECORD_FROM_LENGTH(*it);
                const char *content = RECORD_CONTENT(*it);
                size_t contentSize = RECORD_CONTENT_LENGTH(*it);
                MemBuffer data;
                if (!_cryptoLibrary->unseal(from, fromLen, content, contentSize, &data)) {
                    for (auto *&x : set) {
                        delete x;
                    }
                    set.clear();
                    return false;
                }
                // save the record in a set to reduce loops in other steps.
                const char *innerContent = CIoTA_RECORD_CONTENT(data.data());
                set.push_back(new MemBuffer(innerContent, CIoTA_RECORD_CONTENT_LENGTH(data.data())));
            }
            return true;
        }


        bool testPartialBlock(const char *pBlock, size_t len) override {
            LOG_TRACE("testPartialBlock")
            if (isBlockAcceptable(pBlock, len)) {
                std::vector<MemBuffer *> set;
                collectModels(pBlock, len, set);
                MODEL *model = _utilities->combine(&set);
                for (int i = 0; i < set.size(); i++) {
                    delete set[i];
                }
                set.clear();
                size_t score = _utilities->test(model);
                delete model;
                LOG_TRACE("testPartialBlock score was: {0:d}", score)
                return score >= _alpha;
            }
            return false;
        }


        void notifyOnBlockAcceptance(const char *pBlock, size_t len) override {
            LOG_TRACE("notifyOnBlockAcceptance - Accepted block")
            std::vector<MemBuffer *> set;
            collectModels(pBlock, len, set);
            MODEL *model = _utilities->combine(&set);
            for (int i = 0; i < set.size(); i++) {
                delete set[i];
            }
            _utilities->accept(model);
            if (_listener != nullptr) {
                _listener->onBlockAcceptance(pBlock, len);
            }
        }

        void notifyOnPartialBlockAcceptance(MemBuffer *partialBlock) override {
            LOG_TRACE("notifyOnBlockAcceptance - Accepted partial block {0:d} {1}",
                      BLOCK_NUM_OF_RECORDS(partialBlock->data(), H),
                      BLOCK_SEED(partialBlock->data(), H))
            if (_listener != nullptr) {
                _listener->onPartialBlockAcceptance(partialBlock->data(), partialBlock->size());
            }
        }

        void notifyOnPartialBlockAcceptance(MemBuffer *partialBlock, bool selfUpdate) override {
            if (_listener != nullptr) {
                _listener->onPartialBlockAcceptance(partialBlock->data(), partialBlock->size(), selfUpdate);
            }
        }

        /**
         * For efficient implementation, this function is used to notify on a start of a transaction that about to occur.
         * Its primary use is to save repeated computations.
         */
        virtual void startTransaction() {

        }

        /**
         * For efficient implementation, this function is used to notify on an end of a transaction that about to occur.
         * Its primary use is to save repeated computations.
         */
        virtual void endTransaction() {
            //startTransaction();
        }

        void setListener(BlockchainListener *listener) {
            _listener = listener;
        }
    };
}

#endif //CIOTA_COMPLETE_PROJECT_BASE_CIOTA_BLOCKCHAIN_APPLICATION_H
