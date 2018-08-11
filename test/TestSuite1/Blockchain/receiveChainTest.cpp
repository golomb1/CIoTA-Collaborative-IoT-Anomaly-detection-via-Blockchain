//
// Created by golombt on 20/12/2017.
//
#include "receiveChainTest.h"
#include "../../../headers/Blockchain/Blockchain.h"

#define RECORD_SEED_LENGTH(r) (*((size_t*)(RECORD_CONTENT(r))))
#define RECORD_SEED(record) (RECORD_CONTENT(record) + sizeof(size_t))
#define RECORD_INNER_CONTENT(record) RECORD_SEED(record) + 10



/**
 * @test acceptance of chain, partial block grow and entire chain, between 4 participants.
 * @param app - application instance.
 */
bool case1(BlockchainApplication<TEST_HASH_SIZE> *app, ChainGenerator) {
    std::cout << "\tcase1\n";
    int fromIndex =0;
    Blockchain<TEST_HASH_SIZE>* blockchainList[5];
    BlockchainApplication<TEST_HASH_SIZE>* apps[5];
    MemBuffer* ids[5];
    for (int i=0; i < 5; i++) {
        const std::string &s = std::string("From") + std::to_string(fromIndex);
        ids[i] = new MemBuffer(s.data(), s.size());
        apps[i] = app->copy(ids[i]);
        blockchainList[i] = new Blockchain<TEST_HASH_SIZE>(1024*10, apps[i]);
        fromIndex++;
    }
    blockchainList[0]->updatePartialBlock();
    MemBuffer* partialBlock;
    for(int j=0; j < 3; j++) {
        for (int i = 1; i < 3; i++) {
            blockchainList[i - 1]->consumeChain([blockchainList, i, j](const SerializableBoundedQueue *chain) {
                int res = blockchainList[i]->receiveChain(chain);
                if(j == 0) {
                    ASSERT(res, 0);
                }
                else{
                    ASSERT(res, 1);
                }
                res = blockchainList[i]->receiveChain(chain);
                ASSERT(res, 0);
            });
            partialBlock = blockchainList[i]->updatePartialBlock();
            ASSERT(BLOCK_NUM_OF_RECORDS(partialBlock->data(), TEST_HASH_SIZE), i + 1);
        }
        blockchainList[2]->consumeChain([blockchainList, j](const SerializableBoundedQueue *chain) {
            int res = blockchainList[3]->receiveChain(chain);
            ASSERT(res, 1);
        });
        partialBlock = blockchainList[3]->updatePartialBlock();
        ASSERT(BLOCK_NUM_OF_RECORDS(partialBlock->data(), TEST_HASH_SIZE), 1);
        blockchainList[3]->consumeChain([blockchainList, j](SerializableBoundedQueue *chain) {
            int res = blockchainList[0]->receiveChain(chain);
            ASSERT(res, 1);
        });
        ASSERT(BLOCK_NUM_OF_RECORDS(partialBlock->data(), TEST_HASH_SIZE), 1);
    }
    for (int i=0; i < 5; i++) {
        delete ids[i] ;
        delete apps[i] ;
        delete blockchainList[i];
    }
    return true;
}



/**
 * @test acceptance of chain from participant without a chain.
 * @param app - application instance.
 */
bool case2(BlockchainApplication<TEST_HASH_SIZE> *app, ChainGenerator chainGenerator) {
    std::cout << "\tcase2\n";
    SerializableBoundedQueue *chain = chainGenerator(app, 4, false);
    MemBuffer from("From9", strlen("From9"));
    BlockchainApplication<TEST_HASH_SIZE>* app2 = app->copy(&from);
    Blockchain<TEST_HASH_SIZE> blockchain(1000, app2);
    int res = blockchain.receiveChain(chain);
    ASSERT(res, chain->size());
    delete chain;
    delete app2;
    return true;
}


/**
 * @test test too many partial blocks.
 * @param app - application instance.
 */
bool case3(BlockchainApplication<TEST_HASH_SIZE> *app, ChainGenerator) {
    std::cout << "\tcase3\n";
    int fromIndex = 0;
    // Too many incomplete blocks.
    Blockchain<TEST_HASH_SIZE>* blockchainArray[3];
    BlockchainApplication<TEST_HASH_SIZE>* apps[3];
    MemBuffer* ids[3];
    for (int i=0; i < 3 ; i++) {
        const std::string &s = std::string("From") + std::to_string(fromIndex);
        ids[i] = new MemBuffer(s.data(), s.size());
        apps[i] = app->copy(ids[i]);
        blockchainArray[i] = new Blockchain<TEST_HASH_SIZE>(1024*10, apps[i]);
        fromIndex++;
    }
    MemBuffer *partial = blockchainArray[0]->updatePartialBlock();
    SerializableBoundedQueue queue(1024*1024);
    queue.push_back(partial->data(), partial->size());

    partial = blockchainArray[1]->updatePartialBlock();
    queue.push_back(partial->data(), partial->size());

    int res = blockchainArray[2]->receiveChain(&queue);
    ASSERT(res, VIOLATION_TOO_MANY_PARTIAL_BLOCKS);
    for (int i=0; i < 3; i++) {
        delete ids[i] ;
        delete apps[i] ;
        delete blockchainArray[i];
    }
    return true;
}



/**
 * @test test record with wrong seed inside a block.
 * @param app - application instance.
 */
bool case4(BlockchainApplication<TEST_HASH_SIZE> *app, ChainGenerator chainGenerator){
    std::cout << "\tcase4\n";
    MemBuffer from("From9", strlen("From9"));
    BlockchainApplication<TEST_HASH_SIZE>* app2 = app->copy(&from);
    Blockchain<TEST_HASH_SIZE> blockchain(1024*1024, app2);
    SerializableBoundedQueue *chain = chainGenerator(app, 4, false);
    const char *pBlock = chain->peek(0);
    const char *lastComplete = chain->peek(1);
    size_t partialBlockSize = chain->lengthOfPeek(0);
    size_t lastCompleteSize = chain->lengthOfPeek(1);
    chain->removeHead();
    chain->removeHead();
    MemBuffer newLast(lastComplete, lastCompleteSize);
    MemBuffer newPartial(pBlock, partialBlockSize);
    // alter chain
    //char *r = BLOCK_DATA(newLast.data(), TEST_HASH_SIZE);
    //RECORD_SEED(r)[4] = '8';
    BLOCK_SEED(newLast.data(), TEST_HASH_SIZE)[1] = '8';
    //addRecordContent<TEST_HASH_SIZE>(&newLast, r, RECORD_SIZE(r));

    char h[TEST_HASH_SIZE];
    std::string sha = sha256(hexStr(newLast.data(), newLast.size()));
    memset(h, 0, TEST_HASH_SIZE);
    memcpy(h, sha.data(), sha.size());
    setBlockPrevHash<TEST_HASH_SIZE>(&newPartial,h);
    chain->push_back(newLast.data(), newLast.size());
    chain->push_back(newPartial.data(), newPartial.size());

    auto * partBlock = new MemBuffer(blockchain.updatePartialBlock());
    int res = blockchain.receiveChain(chain);
    ASSERT(res, chain->size() - 2);
    ASSERT( partBlock->size() == blockchain.getPartialBlock()->size() &&
            memcmp(partBlock->data(), blockchain.getPartialBlock()->data(), partBlock->size()) == 0, true);
    delete partBlock;
    delete chain;
    delete app2;
    return true;
}

/**
 * @test test block that includes two record with the same sender (unacceptable block).
 * @param app - application instance.
 */
bool case5(BlockchainApplication<TEST_HASH_SIZE> *app, ChainGenerator chainGenerator){
    std::cout << "\tcase5\n";
    MemBuffer from("From9", strlen("From9"));
    BlockchainApplication<TEST_HASH_SIZE>* app2 = app->copy(&from);
    Blockchain<TEST_HASH_SIZE> blockchain(1024*1024, app2);
    SerializableBoundedQueue *chain = chainGenerator(app, 4, false);
    const char *pBlock = chain->peek(0);
    const char *lastComplete = chain->peek(1);
    size_t partialBlockSize = chain->lengthOfPeek(0);
    size_t lastCompleteSize = chain->lengthOfPeek(1);
    chain->removeHead();
    chain->removeHead();
    MemBuffer newLast(lastComplete, lastCompleteSize);
    MemBuffer newPartial(pBlock, partialBlockSize);
    // alter chain
    // unaccepted block (two identical senders)
    char *r0 = BLOCK_DATA(newLast.data(), TEST_HASH_SIZE);
    const char *r1 = getBlockRecord<TEST_HASH_SIZE>(newLast.data(), 1);
    memcpy(RECORD_FROM(r0), RECORD_FROM(r1), RECORD_FROM_LENGTH(r1));
    //addRecordContent<TEST_HASH_SIZE>(&newLast, r, RECORD_SIZE(r));

    char h[TEST_HASH_SIZE];
    std::string sha = sha256(hexStr(newLast.data(), newLast.size()));
    memset(h, 0, TEST_HASH_SIZE);
    memcpy(h, sha.data(), sha.size());
    setBlockPrevHash<TEST_HASH_SIZE>(&newPartial,h);
    chain->push_back(newLast.data(), newLast.size());
    chain->push_back(newPartial.data(), newPartial.size());
    auto * partBlock = new MemBuffer(blockchain.updatePartialBlock());
    int res = blockchain.receiveChain(chain);
    ASSERT(res, chain->size() - 2);
    ASSERT( partBlock->size() == blockchain.getPartialBlock()->size() &&
            memcmp(partBlock->data(), blockchain.getPartialBlock()->data(), partBlock->size()) == 0, true);
    delete partBlock;
    delete chain;
    delete app2;
    return true;
}



/**
 * @test test partial block that includes two record with the same sender (unacceptable partial block).
 * @param app - application instance.
 */
bool case6(BlockchainApplication<TEST_HASH_SIZE> *app, ChainGenerator chainGenerator ){
    std::cout << "\tcase6\n";
    MemBuffer from("From9", strlen("From9"));
    BlockchainApplication<TEST_HASH_SIZE>* app2 = app->copy(&from);
    Blockchain<TEST_HASH_SIZE> blockchain(1024*1024, app2);
    SerializableBoundedQueue *chain = chainGenerator(app, 4, false);
    const char *pBlock = chain->peek(0);
    const char *lastComplete = chain->peek(1);
    size_t partialBlockSize = chain->lengthOfPeek(0);
    size_t lastCompleteSize = chain->lengthOfPeek(1);
    chain->removeHead();
    chain->removeHead();
    MemBuffer newLast(lastComplete, lastCompleteSize);
    MemBuffer newPartial(pBlock, partialBlockSize);
    // alter chain
    // unaccepted partial block (two identical senders)
    char *r0 = BLOCK_DATA(newPartial.data(), TEST_HASH_SIZE);
    const char *r1 = getBlockRecord<TEST_HASH_SIZE>(newPartial.data(), 1);
    memcpy(RECORD_FROM(r0), RECORD_FROM(r1), RECORD_FROM_LENGTH(r1));
    //addRecordContent<TEST_HASH_SIZE>(&newPartial, r, RECORD_LENGTH(r));

    char h[TEST_HASH_SIZE];
    std::string sha = sha256(hexStr(newLast.data(), newLast.size()));
    memset(h, 0, TEST_HASH_SIZE);
    memcpy(h, sha.data(), sha.size());
    setBlockPrevHash<TEST_HASH_SIZE>(&newPartial,h);
    chain->push_back(newLast.data(), newLast.size());
    chain->push_back(newPartial.data(), newPartial.size());
    auto * partBlock = new MemBuffer(blockchain.updatePartialBlock());
    int res = blockchain.receiveChain(chain);
    ASSERT(res, chain->size() - 1);
    ASSERT( partBlock->size() == blockchain.getPartialBlock()->size() &&
            memcmp(partBlock->data(), blockchain.getPartialBlock()->data(), partBlock->size()) == 0, true);
    delete partBlock;
    delete chain;
    delete app2;
    return true;
}


/**
 * @test test chain which its last block which is not partial includes two record with the same sender (unacceptable block).
 * @param app - application instance.
 */
bool case7(BlockchainApplication<TEST_HASH_SIZE> *app, ChainGenerator chainGenerator){
    std::cout << "\tcase7\n";
    MemBuffer from("From9", strlen("From9"));
    BlockchainApplication<TEST_HASH_SIZE>* app2 = app->copy(&from);
    Blockchain<TEST_HASH_SIZE> blockchain(1024*1024, app2);
    SerializableBoundedQueue *chain = chainGenerator(app, 4, false);
    const char *lastComplete = chain->peek(1);
    size_t lastCompleteSize = chain->lengthOfPeek(1);
    chain->removeHead();
    chain->removeHead();
    MemBuffer newLast(lastComplete, lastCompleteSize);
    // alter chain
    // unaccepted partial block (two identical senders)
    //const char *r = getBlockRecord<TEST_HASH_SIZE>(newLast.data(), 0);
    //addRecordContent<TEST_HASH_SIZE>(&newLast, r, RECORD_LENGTH(r));
    char *r0 = BLOCK_DATA(newLast.data(), TEST_HASH_SIZE);
    const char *r1 = getBlockRecord<TEST_HASH_SIZE>(newLast.data(), 1);
    memcpy(RECORD_FROM(r0), RECORD_FROM(r1), RECORD_FROM_LENGTH(r1));

    std::string sha = sha256(hexStr(newLast.data(), newLast.size()));
    chain->push_back(newLast.data(), newLast.size());
    auto * partBlock = new MemBuffer(blockchain.updatePartialBlock());
    int res = blockchain.receiveChain(chain);
    ASSERT(res, chain->size() - 1);
    ASSERT( partBlock->size() == blockchain.getPartialBlock()->size() &&
            memcmp(partBlock->data(), blockchain.getPartialBlock()->data(), partBlock->size()) == 0, true);
    delete partBlock;
    delete app2;
    delete chain;
    return true;
}


/**
 * @test test chain with wrong hash pointer.
 * @param app - application instance.
 */
bool case8(BlockchainApplication<TEST_HASH_SIZE> *app, ChainGenerator chainGenerator){
    /*
    TODO
     std::cout << "\tcase8\n";
    MemBuffer from("From9", strlen("From9"));
    BlockchainApplication<TEST_HASH_SIZE>* app2 = app->copy(&from);
    Blockchain<TEST_HASH_SIZE> blockchain(1024*1024, app2);
    SerializableBoundedQueue *chain = chainGenerator(app, 4, false);
    const char *pBlock = chain->peek(0);
    const char *lastComplete = chain->peek(1);
    size_t partialBlockSize = chain->lengthOfPeek(0);
    size_t lastCompleteSize = chain->lengthOfPeek(1);
    chain->removeHead();
    chain->removeHead();
    MemBuffer newLast(lastComplete, lastCompleteSize);
    MemBuffer newPartial(pBlock, partialBlockSize);
    // alter chain
    // wrong hash
    {
        char h[TEST_HASH_SIZE];
        memset(h, '1', TEST_HASH_SIZE);
        h[TEST_HASH_SIZE - 1] = 0;
        setBlockPrevHash<TEST_HASH_SIZE>(&newLast, h);
    }
    char h[TEST_HASH_SIZE];
    std::string sha = sha256(hexStr(newLast.data(), newLast.size()));
    memset(h, 0, TEST_HASH_SIZE);
    memcpy(h, sha.data(), sha.size());
    setBlockPrevHash<TEST_HASH_SIZE>(&newPartial,h);
    chain->push_back(newLast.data(), newLast.size());
    chain->push_back(newPartial.data(), newPartial.size());
    auto * partBlock = new MemBuffer(blockchain.updatePartialBlock());
    int res = blockchain.receiveChain(chain);
    ASSERT(res, VIOLATION_WRONG_HASH);
    ASSERT( partBlock->size() == blockchain.getPartialBlock()->size() &&
            memcmp(partBlock->data(), blockchain.getPartialBlock()->data(), partBlock->size()) == 0, true);
    delete partBlock;
    delete chain;
    delete app2;*/
    return true;
}


/**
 * @test test chain with wrong hash pointer in the partial block.
 * @param app - application instance.
 */
bool case9(BlockchainApplication<TEST_HASH_SIZE> *app, ChainGenerator chainGenerator){
    /*
    TODO restore this test
     std::cout << "\tcase9\n";
    MemBuffer from("From9", strlen("From9"));
    BlockchainApplication<TEST_HASH_SIZE>* app2 = app->copy(&from);
    Blockchain<TEST_HASH_SIZE> blockchain(1024*1024, app2);
    SerializableBoundedQueue *chain = chainGenerator(app, 4, false);
    const char *pBlock = chain->peek(0);
    const char *lastComplete = chain->peek(1);
    size_t partialBlockSize = chain->lengthOfPeek(0);
    size_t lastCompleteSize = chain->lengthOfPeek(1);
    chain->removeHead();
    chain->removeHead();
    MemBuffer newLast(lastComplete, lastCompleteSize);
    MemBuffer newPartial(pBlock, partialBlockSize);
    // alter chain
    char h[TEST_HASH_SIZE];
    memset(h, '1', TEST_HASH_SIZE);
    h[TEST_HASH_SIZE - 1]=0;
    setBlockPrevHash<TEST_HASH_SIZE>(&newPartial,h);
    chain->push_back(newLast.data(), newLast.size());
    chain->push_back(newPartial.data(), newPartial.size());
    auto * partBlock = new MemBuffer(blockchain.updatePartialBlock());
    int res = blockchain.receiveChain(chain);
    ASSERT(res, VIOLATION_WRONG_HASH);
    ASSERT( partBlock->size() == blockchain.getPartialBlock()->size() &&
            memcmp(partBlock->data(), blockchain.getPartialBlock()->data(), partBlock->size()) == 0, true);
    delete partBlock;
    delete chain;
    delete app2;
     */
    return true;
}


/**
 * @test chain that start with missing block at the start compare to the current chain.
 * @param app - application instance.
 */
bool case10(BlockchainApplication<TEST_HASH_SIZE> *app, ChainGenerator chainGenerator){
    std::cout << "\tcase10\n";
    MemBuffer from("From9", strlen("From9"));
    BlockchainApplication<TEST_HASH_SIZE>* app2 = app->copy(&from);
    Blockchain<TEST_HASH_SIZE> blockchain(1024*1024, app2);
    SerializableBoundedQueue *chain = chainGenerator(app, 5, true);
    const char *pBlock = chain->peek(0);
    const char *lastComplete = chain->peek(1);
    size_t partialBlockSize = chain->lengthOfPeek(0);
    size_t lastCompleteSize = chain->lengthOfPeek(1);
    chain->removeHead();
    chain->removeHead();
    MemBuffer newLast(lastComplete, lastCompleteSize);
    MemBuffer newPartial(pBlock, partialBlockSize);
    // alter chain
    char h[TEST_HASH_SIZE];
    std::string sha = sha256(hexStr(newLast.data(), newLast.size()));
    memset(h, 0, TEST_HASH_SIZE);
    memcpy(h, sha.data(), sha.size());
    setBlockPrevHash<TEST_HASH_SIZE>(&newPartial,h);
    chain->push_back(newLast.data(), newLast.size());
    chain->push_back(newPartial.data(), newPartial.size());
    blockchain.receiveChain(chain);
    auto * partBlock = new MemBuffer(blockchain.updatePartialBlock());
    chain->removeTail();
    int res = blockchain.receiveChain(chain);
    ASSERT(res, 0);
    ASSERT( partBlock->size() == blockchain.getPartialBlock()->size() &&
            memcmp(partBlock->data(), blockchain.getPartialBlock()->data(), partBlock->size()) == 0, true);
    delete partBlock;
    delete app2;
    delete chain;
    return true;
}



/**
 * @test collision with longer chain.
 * @param app - application instance.
 */
bool case11(BlockchainApplication<TEST_HASH_SIZE>* app, ChainGenerator chainGenerator){
    std::cout << "\tcase11\n";
    MemBuffer from("From9", strlen("From9"));
    BlockchainApplication<TEST_HASH_SIZE>* app2 = app->copy(&from);
    Blockchain<TEST_HASH_SIZE> blockchain(1024*1024, app2);
    SerializableBoundedQueue *chain = chainGenerator(app, 6, false);
    SerializableBoundedQueue *chain1 = extendChain(app, chain, 2, 0, false);
    SerializableBoundedQueue *chain2 = extendChain(app, chain, 4, 8, false);
    int l = blockchain.receiveChain(chain1);
    ASSERT(l, 8);
    int res = blockchain.receiveChain(chain2);
    ASSERT(res, 4);
    blockchain.consumeChain([chain2](SerializableBoundedQueue *chain) {
        const char *cSeed  = BLOCK_SEED(chain->peek(1), TEST_HASH_SIZE);
        const char *cSeed2 = BLOCK_SEED(chain2->peek(1), TEST_HASH_SIZE);
        size_t seedLen = BLOCK_SEED_SIZE(chain->peek(1), TEST_HASH_SIZE);
        ASSERT(memcmp(cSeed,cSeed2,seedLen), 0);
    });
    delete chain1;
    delete chain2;
    delete chain;
    delete app2;
    return true;
}



/**
 * @test test chain with wrong index.
 * @param app - application instance.
 */
bool case12(BlockchainApplication<TEST_HASH_SIZE> *app, ChainGenerator chainGenerator){
    std::cout << "\tcase12\n";
    MemBuffer from("From9", strlen("From9"));
    BlockchainApplication<TEST_HASH_SIZE>* app2 = app->copy(&from);
    Blockchain<TEST_HASH_SIZE> blockchain(1024*1024, app2);
    SerializableBoundedQueue *chain = chainGenerator(app, 4, false);
    const char *pBlock = chain->peek(0);
    const char *lastComplete = chain->peek(1);
    size_t partialBlockSize = chain->lengthOfPeek(0);
    size_t lastCompleteSize = chain->lengthOfPeek(1);
    chain->removeHead();
    chain->removeHead();
    MemBuffer newLast(lastComplete, lastCompleteSize);
    MemBuffer newPartial(pBlock, partialBlockSize);
    // alter chain
    // wrong hash
    {
        setBlockId<TEST_HASH_SIZE>(&newLast, 26);
    }
    char h[TEST_HASH_SIZE];
    std::string sha = sha256(hexStr(newLast.data(), newLast.size()));
    memset(h, 0, TEST_HASH_SIZE);
    memcpy(h, sha.data(), sha.size());
    setBlockPrevHash<TEST_HASH_SIZE>(&newPartial,h);
    chain->push_back(newLast.data(), newLast.size());
    chain->push_back(newPartial.data(), newPartial.size());
    auto * partBlock = new MemBuffer(blockchain.updatePartialBlock());
    int res = blockchain.receiveChain(chain);
    ASSERT(res, VIOLATION_WRONG_INDEX);
    ASSERT( partBlock->size() == blockchain.getPartialBlock()->size() &&
            memcmp(partBlock->data(), blockchain.getPartialBlock()->data(), partBlock->size()) == 0, true);
    delete partBlock;
    delete app2;
    delete chain;
    return true;
}


/**
 * @test test chain with wrong index in the partial block.
 * @param app - application instance.
 */
bool case13(BlockchainApplication<TEST_HASH_SIZE> *app, ChainGenerator chainGenerator){
    std::cout << "\tcase13\n";
    MemBuffer from("From9", strlen("From9"));
    BlockchainApplication<TEST_HASH_SIZE>* app2 = app->copy(&from);
    Blockchain<TEST_HASH_SIZE> blockchain(1024*1024, app2);
    SerializableBoundedQueue *chain = chainGenerator(app, 4, false);
    const char *pBlock = chain->peek(0);
    const char *lastComplete = chain->peek(1);
    size_t partialBlockSize = chain->lengthOfPeek(0);
    size_t lastCompleteSize = chain->lengthOfPeek(1);
    chain->removeHead();
    chain->removeHead();
    MemBuffer newLast(lastComplete, lastCompleteSize);
    MemBuffer newPartial(pBlock, partialBlockSize);
    // alter chain
    char h[TEST_HASH_SIZE];
    std::string sha = sha256(hexStr(newLast.data(), newLast.size()));
    memset(h, 0, TEST_HASH_SIZE);
    memcpy(h, sha.data(), sha.size());
    setBlockPrevHash<TEST_HASH_SIZE>(&newPartial,h);
    {
        setBlockId<TEST_HASH_SIZE>(&newPartial, 26);
    }
    chain->push_back(newLast.data(), newLast.size());
    chain->push_back(newPartial.data(), newPartial.size());
    auto * partBlock = new MemBuffer(blockchain.updatePartialBlock());
    int res = blockchain.receiveChain(chain);
    ASSERT(res, VIOLATION_WRONG_INDEX);
    ASSERT( partBlock->size() == blockchain.getPartialBlock()->size() &&
            memcmp(partBlock->data(), blockchain.getPartialBlock()->data(), partBlock->size()) == 0, true);
    delete partBlock;
    delete app2;
    delete chain;
    return true;
}

/**
 * @test test shorter chain.
 * @param app - application instance.
 */
bool case14(BlockchainApplication<TEST_HASH_SIZE> *app, ChainGenerator chainGenerator){
    std::cout << "\tcase14\n";
    MemBuffer from("From9", strlen("From9"));
    BlockchainApplication<TEST_HASH_SIZE>* app2 = app->copy(&from);
    Blockchain<TEST_HASH_SIZE> blockchain(1024*1024, app2);
    SerializableBoundedQueue *chain1 = chainGenerator(app, 6, false);
    SerializableBoundedQueue *chain2 = chainGenerator(app, 4, true);
    ASSERT(blockchain.receiveChain(chain1), 6);
    ASSERT(blockchain.receiveChain(chain2), VIOLATION_OLD_CHAIN);
    delete app2;
    delete chain1;
    delete chain2;
    return true;
}

bool receiveChainTest(BlockchainApplication<TEST_HASH_SIZE>* app,
                      ChainGenerator chainGenerator)
{
    std::cout << "receiveChainTest\n";
    return case1(app, chainGenerator) &&
    case2(app,chainGenerator) &&
    case3(app,chainGenerator) &&
    case4(app,chainGenerator) &&
    case5(app,chainGenerator) &&
    case6(app,chainGenerator) &&
    case7(app,chainGenerator) &&
    case8(app,chainGenerator) &&
    case9(app,chainGenerator) &&
    case10(app,chainGenerator) &&
    case11(app,chainGenerator) &&
    case12(app,chainGenerator) &&
    case13(app,chainGenerator) &&
    case14(app,chainGenerator);
}