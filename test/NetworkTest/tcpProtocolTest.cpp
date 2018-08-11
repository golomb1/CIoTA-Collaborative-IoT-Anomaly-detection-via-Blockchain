//
// Created by master on 16/01/18.
//

#include <atomic>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <netinet/in.h>
#include "NetworkTestMethods.h"
#include "../TEST.h"
#include "../../headers/NetworkingUtils/CompressionLibrary.h"
#include "../../headers/NetworkingUtils/MessageFragmentationServer.h"
#include "../../headers/NetworkingUtils/SimpleTcpProtocol/SimpleTcpProtocolServer.h"
#include "../../headers/NetworkingUtils/SimpleTcpProtocol/SimpleTcpProtocolClient.h"

#define TEST_PORT      12346
#define BUFFER_SIZE    64
#define MESSAGE        "TEST MESSAGE"
#define RESPONSE       "TEST RESPONSE"
#define TEST_MSG_TYPE  1


void testTcpProtocolClientMain() {
    std::cout << "------------------------------" << std::endl;
    std::cout << "Start TCP Protocol test, " << std::endl;
    std::cout << "Please be sure to execute NetworkingUtilTests as a server on a different machine." << std::endl;

    SimpleTcpProtocolClient client;
    std::cout << "Please enter the server ip address: " << std::endl;
    std::string ip;
    std::cin >> ip;
    std::cout << "Try to send protocol message to a machine at " << ip << std::endl;

    client.startSession(ip.c_str(), TEST_PORT);
    size_t outLen;
    char *out = client.send(TEST_MSG_TYPE, MESSAGE, strlen(MESSAGE), &outLen);
    std::cout << "Wait for the server's response..." << std::flush;

    ASSERT(outLen, strlen(RESPONSE));
    ASSERT(strncmp(out, RESPONSE, outLen), 0);
    delete out;

    std::cout << "Received." << std::endl;
}

char *helloHandler(std::atomic<bool> *flag, sockaddr_in const *, const char *buffer, size_t len, size_t *outLen) {
    ASSERT(len, strlen(MESSAGE));
    ASSERT(strncmp(buffer, MESSAGE, len), 0);
    auto *out = new char[strlen(RESPONSE)];
    *outLen = strlen(RESPONSE);
    memcpy(out, RESPONSE, strlen(RESPONSE));
    std::cout << "The test success, please press any key to continue to exit." << std::endl;
    flag->store(true);
    return out;
}

void testTcpProtocolServerMain() {
    std::cout << "------------------------------" << std::endl;
    std::cout << "Start TCP Protocol test, " << std::endl;
    std::cout << "Please be sure to execute NetworkingUtilTests as a client on a different machine." << std::endl;
    std::atomic<bool> flag(false);
    SimpleTcpProtocolServer<std::atomic<bool>> server;
    server.setMetadata(&flag);
    server.addHandle(TEST_MSG_TYPE, helloHandler);
    server.run(TEST_PORT);
    std::cout << "Start executing the server..." << std::endl;
    do {
        char key;
        std::cin >> key;
    } while (!flag.load());
    server.stop();
}