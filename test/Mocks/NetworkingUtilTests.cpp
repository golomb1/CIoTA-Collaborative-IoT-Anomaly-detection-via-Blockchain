//
// Created by golombt on 20/12/2017.
//
#include "../../headers/NetworkingUtils/CompressionLibrary.h"
#include "../../headers/NetworkingUtils/MessageFragmentationServer.h"
#include "../../headers/CppUtils/SerializableBoundedQueue.h"
#include "NetworkingUtilTests.h"
#include "BlockchainTestUtils.h"
#include "../TEST.h"
#include <chrono>
#include <thread>

#define RECEIVE_BLOCK_TYPE 1


bool BroadcastTest(){
    std::cout << "BroadcastTest" << std::endl;
    FileDescriptorWrapperTest manager;
    MockCompressionLibrary libraryTest;
    MessageFragmentationServer<void> server(512, &libraryTest, &manager);

    MemBuffer id;
    id.append("ID", 2);
    BlockchainApplicationTest<TEST_HASH_SIZE> app(&id);
    SerializableBoundedQueue *chain = generateChain2(&app, 10, true);

    server.addHandler(RECEIVE_BLOCK_TYPE, [chain](void* md, const char* buffer, size_t len){
        ASSERT(len, chain->serializationSize());
        ASSERT(memcmp(buffer, chain->serialize(), len), 0);
        return true;
    });

    server.broadcastFragmentation(9999, RECEIVE_BLOCK_TYPE, chain->serialize(), chain->serializationSize());
    while(!server.receiveFragmentation()){
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    delete chain;
    return true;
}


int FileDescriptorWrapperTest::recv(char *buffer, size_t len, char *) {
    MemBuffer b = _queue.at(0);
    _queue.erase(_queue.begin(), _queue.begin() + 1);
    size_t min = b.size();
    if (min > len) {
        min = len;
    }
    memcpy(buffer, b.data(), min);
    return static_cast<int>(min);
}

int FileDescriptorWrapperTest::broadcast(unsigned short, const char *buffer, size_t len) {
    _queue.emplace_back(buffer, len);
}

bool FileDescriptorWrapperTest::awaitForMessage() {
    while (_queue.empty()) {}
}
