//
// Created by golombt on 20/12/2017.
//

#ifndef TEST_NETWORKING_UTIL_TESTS_H
#define TEST_NETWORKING_UTIL_TESTS_H

#include "../../headers/NetworkingUtils/CompressionLibrary.h"
#include "../../headers/NetworkingUtils/BroadcastServer.h"

class FileDescriptorWrapperTest : public BroadcastServer {
public:
    int recv(char *buffer, size_t len, char *fromAddress) override;

    int broadcast(unsigned short port, const char *buffer, size_t len) override;

    bool awaitForMessage() override;

    std::vector<MemBuffer> _queue;


};

bool BroadcastTest();


#endif //TEST_NETWORKING_UTIL_TESTS_H
