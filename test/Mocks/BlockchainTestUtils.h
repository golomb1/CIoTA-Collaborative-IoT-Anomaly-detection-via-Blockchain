//
// Created by golombt on 20/12/2017.
//

#ifndef TEST_BLOCKCHAIN_TEST_H
#define TEST_BLOCKCHAIN_TEST_H

#include <sstream>
#include <iomanip>
#include "../../headers/Blockchain/BlockRecord.h"
#include "../../headers/Blockchain/BlockchainApplication.h"
#include "../../headers/CppUtils/SHA256.h"
#include "../../headers/CppUtils/SerializableBoundedQueue.h"

#define TEST_RECORD_FROM_SIZE 30
#define TEST_RECORD_LENGTH 30
#define TEST_HASH_SIZE     SHA256_OUTPUT_SIZE

std::string hexStr(const char *data, size_t len);


typedef SerializableBoundedQueue* (*ChainGenerator)(BlockchainApplication<TEST_HASH_SIZE>*, int , bool );
typedef BlockRecord* (*RecordGenerator)(int fromIndex,int seedIndex);
typedef SerializableBoundedQueue* (*GenerateCollidingChain)(BlockchainApplication<TEST_HASH_SIZE>*, int length, int length1, int k, bool createPartial);

/**
 * Helper function that generate a record for specific participant with specific seed.
 * @param fromIndex is the participant number
 * @param seedIndex is the seed number
 * @return the record.
 */
BlockRecord *getRecord2(int fromIndex,int seedIndex) ;



template <unsigned short H>
class BlockchainApplicationTest : public BlockchainApplication<H> {
private:
    MemBuffer* _id2;
    int seed;
    int contentId = 0;
public:
    explicit BlockchainApplicationTest(MemBuffer* id) : BlockchainApplication<H>(id, true), _id2(id) , seed(0) {}

    bool isCompletedBlock(const char *block, size_t len) override {
        size_t l = BLOCK_NUM_OF_RECORDS(block, H);
        bool r = l == 3;
        return r;
    }

    void getHash(char *string, const char *pBlock, size_t len) override {
        std::string sha = sha256(hexStr(pBlock, len));
        memset(string, 0, H);
        memcpy(string, sha.data(), sha.size());
    }

    bool isBlockAcceptable(const char *pBlock, size_t len) override {
        if(BLOCK_NUM_OF_RECORDS(pBlock, H) == 0){
            return false;
        }
        BlockRecordIterator it = blockRecordsBegin<H>(pBlock);
        BlockRecordIterator end = blockRecordsEnd<H>(pBlock);
        size_t seedLength = BLOCK_SEED_SIZE(pBlock, H);
        const char* seed = BLOCK_SEED(pBlock, H);
        for(;it != end; ++it){
            BlockRecordIterator innerIt = it;
            innerIt++;
            for(;innerIt != end; ++innerIt) {
                if(RECORD_FROM_LENGTH(*innerIt) == RECORD_FROM_LENGTH(*it)
                   && memcmp(RECORD_FROM(*innerIt), RECORD_FROM(*it), RECORD_FROM_LENGTH(*innerIt)) == 0)
                {
                    return false;
                }
            }
            const char *s = RECORD_CONTENT(*it) + sizeof(size_t);
            size_t sL = *((size_t*)(RECORD_CONTENT(*it)));
            if(seedLength != sL || memcmp(seed, s, seedLength) != 0){
                return false;
            }
        }
        return true;
    }

    bool testPartialBlock(const char *pBlock, size_t len) override {
        return isBlockAcceptable(pBlock, len);
    }

    void notifyOnBlockAcceptance(const char *pBlock, size_t len) override {

    }

    void notifyOnPartialBlockAcceptance(MemBuffer *partialBlock) override {

    }

    void notifyOnPartialBlockAcceptance(MemBuffer *partialBlock, bool selfUpdate) override {

    }

    void makeRecord(const char *block, size_t len, MemBuffer *out) override {
        MemBuffer content;
       size_t seedSize = BLOCK_SEED_SIZE(block, H);
        content.append(&seedSize, sizeof(seedSize));
        content.append(BLOCK_SEED(block, H), seedSize);
        const std::string &s = std::string("Content") + std::to_string(contentId);
        content.append(s.data(), s.size());
        BlockRecord record(BlockchainApplication<H>::getFrom(), BlockchainApplication<H>::getFromSize(), content.data(), content.size());
        out->swap(record);
        contentId++;
    }

    void getSeed(size_t blockId, time_t timestamp, const char *prevBlockHash, MemBuffer *out) override {
        const std::string &s = std::string("Seed") + _id2->data()[4] + std::to_string(seed);
        out->append(s.data(), s.size());
        seed++;
    }

    BlockchainApplication<H> *copy(MemBuffer *id) override {
        return new BlockchainApplicationTest(id);
    }
};


/**
 * Generate a chain of ids from zero until length with participant id 0 until 3 includes.
 * @param app application.
 * @param length of the generated chain.
 * @return the chain.
 */
SerializableBoundedQueue* generateChain2(BlockchainApplication<TEST_HASH_SIZE> *app, int length, bool partial);


SerializableBoundedQueue* extendChain(BlockchainApplication<TEST_HASH_SIZE> *app, SerializableBoundedQueue* chain, int length, int k, bool createPartial);


#endif //TEST_BLOCKCHAIN_TEST_H
