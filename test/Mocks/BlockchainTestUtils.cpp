//
// Created by golombt on 20/12/2017.
//

#include "BlockchainTestUtils.h"
#include "../../headers/Blockchain/Blockchain.h"


std::string hexStr(const char *data, size_t len){
    std::stringstream ss;
    for(size_t i(0);i<len;++i) {
        ss << std::hex << (int) (data[i]);
    }
    return ss.str();
}


/**
 * Helper function that generate a record for specific participant with specific seed.
 * @param fromIndex is the participant number
 * @param seedIndex is the seed number
 * @return the record.
 */
BlockRecord *getRecord2(int fromIndex,int seedIndex) {
    char from[TEST_RECORD_FROM_SIZE];
    char cont[TEST_RECORD_LENGTH];
    memset(from, 0, TEST_RECORD_FROM_SIZE);
    memset(cont, 0, TEST_RECORD_LENGTH);
    strcpy(from, "From");
    from[strlen("From")] = static_cast<char>('0' + fromIndex);
    *((size_t*)cont) = 10;
    strcpy((cont + sizeof(size_t)), "Seed");
    cont[sizeof(size_t) + strlen("Seed")] = static_cast<char>('0' + seedIndex);
    strcpy(cont + sizeof(size_t) + 10, "Content");
    cont[sizeof(size_t) + 10 + strlen("Content")] = static_cast<char>('0' + fromIndex);
    auto* res = new BlockRecord(from, TEST_RECORD_FROM_SIZE, cont, TEST_RECORD_LENGTH);
    return res;
}


SerializableBoundedQueue* generateChain2(BlockchainApplication<TEST_HASH_SIZE> *app, int length, bool createPartial){
    Blockchain<TEST_HASH_SIZE>* blockchainList[10];
    BlockchainApplication<TEST_HASH_SIZE>* apps[10];
    MemBuffer* ids[10];

    int fromIndex = 0;
    for(int i=0; i < 10; i++ ) {
        const std::string &s = std::string("From") + std::to_string(fromIndex);
        ids[i] = new MemBuffer(s.data(), s.size());
        apps[i] = app->copy(ids[i]);
        blockchainList[i] = new Blockchain<TEST_HASH_SIZE>(1024*10, apps[i]);
        fromIndex++;
    }
    blockchainList[0]->updatePartialBlock();
    for(int j=0; j < length; j++) {
        for (int i = 1; i < 3; i++) {
            blockchainList[i - 1]->consumeChain([blockchainList, i, j](const SerializableBoundedQueue *chain) {
                blockchainList[i]->receiveChain(chain);
            });
            blockchainList[i]->updatePartialBlock();
        }
        blockchainList[2]->consumeChain([blockchainList, j](SerializableBoundedQueue *chain) {
            blockchainList[3]->receiveChain(chain);
        });
        if(createPartial || j!=length-1) {
            blockchainList[3]->updatePartialBlock();
        }
        blockchainList[3]->consumeChain([blockchainList, j](SerializableBoundedQueue *chain) {
            blockchainList[0]->receiveChain(chain);
        });
    }
    SerializableBoundedQueue* res;
    SerializableBoundedQueue** pRes = &res;
    blockchainList[0]->consumeChain([blockchainList, pRes](SerializableBoundedQueue *chain) {
        *pRes = new SerializableBoundedQueue(chain);
    });
    for(int i=0; i < 10; i++){
        delete ids[i];
        delete blockchainList[i];
        delete(apps[i]);
    }
    return res;
}

SerializableBoundedQueue* extendChain(BlockchainApplication<TEST_HASH_SIZE> *app, SerializableBoundedQueue* chain, int length, int k, bool createPartial) {
    Blockchain<TEST_HASH_SIZE>* blockchainList[10];
    BlockchainApplication<TEST_HASH_SIZE>* apps[10];
    MemBuffer* ids[10];
    int fromIndex = k;
    for (int i=0; i < 10; i++) {
        const std::string &s = std::string("From") + std::to_string(fromIndex);
        ids[i] = new MemBuffer(s.data(), s.size());
        apps[i] = app->copy(ids[i]);
        blockchainList[i] = new Blockchain<TEST_HASH_SIZE>(1024*10, apps[i]);
        blockchainList[i]->receiveChain(chain);
        fromIndex++;
    }
    blockchainList[0]->updatePartialBlock();
    for(int j=0; j < length; j++) {
        for (int i = 1; i < 3; i++) {
            blockchainList[i - 1]->consumeChain([blockchainList, i, j](const SerializableBoundedQueue *chain) {
                blockchainList[i]->receiveChain(chain);
            });
            blockchainList[i]->updatePartialBlock();
        }
        blockchainList[2]->consumeChain([blockchainList, j](SerializableBoundedQueue *chain) {
            blockchainList[3]->receiveChain(chain);
        });
        if(createPartial || j!=length-1) {
            blockchainList[3]->updatePartialBlock();
        }
        blockchainList[3]->consumeChain([blockchainList, j](SerializableBoundedQueue *chain) {
            blockchainList[0]->receiveChain(chain);
        });
    }
    SerializableBoundedQueue* res;
    SerializableBoundedQueue** pRes = &res;
    blockchainList[0]->consumeChain([blockchainList, pRes](SerializableBoundedQueue *chain) {
        *pRes = new SerializableBoundedQueue(chain);
    });
    for (int i=0; i < 10; i++) {
        delete blockchainList[i];
        delete(ids[i]);
        delete(apps[i]);
    }
    return res;
}