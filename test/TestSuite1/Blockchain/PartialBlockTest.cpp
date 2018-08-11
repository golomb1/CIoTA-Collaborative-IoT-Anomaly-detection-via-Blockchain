//
// Created by golombt on 20/12/2017.
//

#include "PartialBlockTest.h"
#include "../../../headers/Blockchain/Blockchain.h"

#define RECORD_SEED_LEN(record) *(size_t*)(RECORD_CONTENT(record))
#define RECORD_SEED(record) (RECORD_CONTENT(record) + sizeof(size_t))
#define RECORD_INNER_CONTENT(record) RECORD_SEED(record) + RECORD_SEED_LEN(record)



/**
 * @test the partial block generation by:
 *    (1) generating new partial block (when non existing) and without chain.
 *    (2) generating new partial block (when non existing) and with chain.
 * @param app - application instance.
 */
bool case1and2(BlockchainApplication<TEST_HASH_SIZE>* &app, ChainGenerator chainGenerator, bool checkSeed){
    std::cout << "\tcase1and2\n";
    for(int i=0; i < 2; i++) {
        char hash[TEST_HASH_SIZE];
        memset(hash, 0, TEST_HASH_SIZE);
        MemBuffer from("From9", strlen("From9"));
        BlockchainApplication<TEST_HASH_SIZE>* copy = app->copy(&from);
        Blockchain<TEST_HASH_SIZE> blockchain(1000, copy);
        if (i == 1) {
            SerializableBoundedQueue *chain = chainGenerator(app, 3, false);
            blockchain.receiveChain(chain);
            app->getHash(hash, chain->head(), chain->headLength());
            delete chain;
        }
        MemBuffer *partialBlock = blockchain.updatePartialBlock();
        ASSERT(BLOCK_ID(partialBlock->data()), 0 + i*3);
        ASSERT(BLOCK_NUM_OF_RECORDS(partialBlock->data(), TEST_HASH_SIZE), 1);


        ASSERT(memcmp(BLOCK_PREV_HASH(partialBlock->data()), hash, TEST_HASH_SIZE), 0);
        ASSERT(RECORD_FROM_LENGTH(getBlockRecord<TEST_HASH_SIZE>(partialBlock->data(), 0)), 5);
        ASSERT(memcmp(RECORD_FROM(getBlockRecord<TEST_HASH_SIZE>(partialBlock->data(), 0)), "From9", strlen("From9")), 0);

        if(checkSeed) {
            ASSERT(memcmp(RECORD_SEED(getBlockRecord<TEST_HASH_SIZE>(partialBlock->data(), 0)), "Seed9",
                          strlen("Seed9")), 0);
            ASSERT(RECORD_SEED(getBlockRecord<TEST_HASH_SIZE>(partialBlock->data(), 0))[strlen("Seed9")], '0');
        }

        ASSERT(memcmp(RECORD_INNER_CONTENT(getBlockRecord<TEST_HASH_SIZE>(partialBlock->data(), 0)), "Content0", strlen("Content0")), 0);
        ASSERT(blockchain.updatePartialBlock(), partialBlock);
        delete(copy);
    }
    return true;
}

/**
 * @test the partial block generation by:
 *    (3) updating the partial block when without a chain.
 *    (4) updating the partial block when with a chain.
 * @param app - application instance.
 */
bool case3and4(BlockchainApplication<TEST_HASH_SIZE>* &app, ChainGenerator chainGenerator, bool checkSeed){
    std::cout << "\tcase3and4\n";
    for(int i=0; i < 2; i++) {
        char hash[TEST_HASH_SIZE];
        memset(hash, 0, TEST_HASH_SIZE);
        MemBuffer from9("From9", strlen("From9"));
        BlockchainApplication<TEST_HASH_SIZE>* copy9 = app->copy(&from9);
        Blockchain<TEST_HASH_SIZE> blockchain0(1000, copy9);
        if (i == 1) {
            SerializableBoundedQueue *chain = chainGenerator(app, 3, false);
            blockchain0.receiveChain(chain);
            app->getHash(hash, chain->head(), chain->headLength());
            delete chain;
        }
        blockchain0.updatePartialBlock();
        MemBuffer from8("From8", strlen("From8"));
        BlockchainApplication<TEST_HASH_SIZE>* copy8 = app->copy(&from8);
        Blockchain<TEST_HASH_SIZE> blockchain1(1000, copy8);
        Blockchain<TEST_HASH_SIZE> *pBlockchain1 = &blockchain1;
        blockchain0.consumeChain([pBlockchain1](SerializableBoundedQueue *chain) {
            pBlockchain1->receiveChain(chain);
        });

        blockchain1.updatePartialBlock();
        MemBuffer from7("From7", strlen("From7"));
        BlockchainApplication<TEST_HASH_SIZE>* copy7 = app->copy(&from7);
        Blockchain<TEST_HASH_SIZE> blockchain2(1000, copy7);
        Blockchain<TEST_HASH_SIZE> *pBlockchain2 = &blockchain2;
        blockchain1.consumeChain([pBlockchain2](SerializableBoundedQueue *chain) {
            pBlockchain2->receiveChain(chain);
        });

        MemBuffer *partialBlock2 = blockchain2.updatePartialBlock();
        ASSERT(BLOCK_ID(partialBlock2->data()), i*3);
        ASSERT(BLOCK_NUM_OF_RECORDS(partialBlock2->data(), TEST_HASH_SIZE), 3);


        ASSERT(memcmp(BLOCK_PREV_HASH(partialBlock2->data()), hash, TEST_HASH_SIZE), 0);
        ASSERT(RECORD_FROM_LENGTH(getBlockRecord<TEST_HASH_SIZE>(partialBlock2->data(), 0)), 5);
        ASSERT(memcmp(RECORD_FROM(getBlockRecord<TEST_HASH_SIZE>(partialBlock2->data(), 0)), "From9", strlen("From9")), 0);
        if(checkSeed) {
            ASSERT(memcmp(RECORD_SEED(getBlockRecord<TEST_HASH_SIZE>(partialBlock2->data(), 0)), "Seed9",
                          strlen("Seed9")), 0);
            ASSERT(RECORD_SEED(getBlockRecord<TEST_HASH_SIZE>(partialBlock2->data(), 0))[strlen("Seed9")], '0');
        }
        ASSERT(memcmp(RECORD_INNER_CONTENT(getBlockRecord<TEST_HASH_SIZE>(partialBlock2->data(), 0)), "Content0", strlen("Content0")),
               0);

        ASSERT(RECORD_FROM_LENGTH(getBlockRecord<TEST_HASH_SIZE>(partialBlock2->data(), 1)), 5);
        ASSERT(memcmp(RECORD_FROM(getBlockRecord<TEST_HASH_SIZE>(partialBlock2->data(), 1)), "From8", strlen("From8")), 0);
        if(checkSeed) {
            ASSERT(memcmp(RECORD_SEED(getBlockRecord<TEST_HASH_SIZE>(partialBlock2->data(), 0)), "Seed9",
                          strlen("Seed9")), 0);
            ASSERT(RECORD_SEED(getBlockRecord<TEST_HASH_SIZE>(partialBlock2->data(), 0))[strlen("Seed9")], '0');
        }
        ASSERT(memcmp(RECORD_INNER_CONTENT(getBlockRecord<TEST_HASH_SIZE>(partialBlock2->data(), 1)), "Content0", strlen("Content0")),
               0);


        ASSERT(RECORD_FROM_LENGTH(getBlockRecord<TEST_HASH_SIZE>(partialBlock2->data(), 2)), 5);
        ASSERT(memcmp(RECORD_FROM(getBlockRecord<TEST_HASH_SIZE>(partialBlock2->data(), 2)), "From7", strlen("From7")), 0);
        if(checkSeed) {
            ASSERT(memcmp(RECORD_SEED(getBlockRecord<TEST_HASH_SIZE>(partialBlock2->data(), 0)), "Seed9",
                          strlen("Seed9")), 0);
            ASSERT(RECORD_SEED(getBlockRecord<TEST_HASH_SIZE>(partialBlock2->data(), 0))[strlen("Seed9")], '0');
        }ASSERT(memcmp(RECORD_INNER_CONTENT(getBlockRecord<TEST_HASH_SIZE>(partialBlock2->data(), 2)), "Content0", strlen("Content0")),
               0);

        ASSERT(blockchain2.updatePartialBlock(), partialBlock2);
        delete(copy9);
        delete(copy8);
        delete(copy7);
    }
    return true;
}


/**
 * @test the partial block generation by:
 *    (5) record already in the partial block when without a chain.
 *    (6) record already in the partial block when with a chain.
 * @param app - application instance.
 */
bool case5and6(BlockchainApplication<TEST_HASH_SIZE>* &app, ChainGenerator chainGenerator, bool checkSeed){
    std::cout << "\tcase5and6\n";
    for(int i=0; i < 2; i++) {
        MemBuffer from9("From9", strlen("From9"));
        BlockchainApplication<TEST_HASH_SIZE>* copy = app->copy(&from9);
        Blockchain<TEST_HASH_SIZE> blockchain(1000, copy);
        char hash[TEST_HASH_SIZE];
        memset(hash, 0, TEST_HASH_SIZE);
        if (i == 1) {
            SerializableBoundedQueue *chain = chainGenerator(app, 3, false);
            blockchain.receiveChain(chain);
            app->getHash(hash, chain->head(), chain->headLength());
            delete chain;
        }
        MemBuffer *partialBlock = blockchain.updatePartialBlock();
        blockchain.updatePartialBlock();
        ASSERT(BLOCK_ID(partialBlock->data()), i*3);
        ASSERT(BLOCK_NUM_OF_RECORDS(partialBlock->data(), TEST_HASH_SIZE), 1);

        ASSERT(memcmp(BLOCK_PREV_HASH(partialBlock->data()), hash, TEST_HASH_SIZE), 0);
        ASSERT(RECORD_FROM_LENGTH(getBlockRecord<TEST_HASH_SIZE>(partialBlock->data(), 0)), 5);
        ASSERT(memcmp(RECORD_FROM(getBlockRecord<TEST_HASH_SIZE>(partialBlock->data(), 0)), "From9", strlen("From9")), 0);
        if(checkSeed) {
            ASSERT(memcmp(RECORD_SEED(getBlockRecord<TEST_HASH_SIZE>(partialBlock->data(), 0)), "Seed9",
                          strlen("Seed9")), 0);
            ASSERT(RECORD_SEED(getBlockRecord<TEST_HASH_SIZE>(partialBlock->data(), 0))[strlen("Seed9")], '0');
        }
        ASSERT(memcmp(RECORD_INNER_CONTENT(getBlockRecord<TEST_HASH_SIZE>(partialBlock->data(), 0)), "Content0", strlen("Content0")), 0);
        ASSERT(blockchain.updatePartialBlock(), partialBlock);
        delete(copy);
    }
    return true;
}




/**
 * @test the partial block generation by:
 *    (1) generating new partial block (when non existing) and without chain.
 *    (2) generating new partial block (when non existing) and with chain.
 *    (3) updating the partial block when without a chain.
 *    (4) updating the partial block when with a chain.
 *    (5) record already in the partial block when without a chain.
 *    (6) record already in the partial block when with a chain.
 * @param app - application instance.
 */
bool TestPartialBlockGeneration(BlockchainApplication<TEST_HASH_SIZE> *app,
                                ChainGenerator chainGenerator, bool checkSeed)
{
    std::cout << "TestPartialBlockGeneration\n";
    return case1and2(app, chainGenerator, checkSeed) &&
            case3and4(app, chainGenerator, checkSeed) &&
            case5and6(app, chainGenerator, checkSeed);
}