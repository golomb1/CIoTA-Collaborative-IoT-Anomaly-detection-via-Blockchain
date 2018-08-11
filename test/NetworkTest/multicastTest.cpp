//
// Created by master on 16/01/18.
//

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include "NetworkTestMethods.h"
#include "../../headers/NetworkingUtils/MulticastServerDescriptor.h"
#include "../TEST.h"
#include "../../headers/NetworkingUtils/CompressionLibrary.h"
#include "../../headers/NetworkingUtils/MessageFragmentationServer.h"

#define TEST_GROUP     "225.0.0.37"
#define TEST_PORT      12345
#define BUFFER_SIZE    64
#define MESSAGE        "TEST MESSAGE"
#define FRAGMENT_SIZE  64
#define TEST_MSG_TYPE  1


void testMulticastClientMain() {
    std::cout << "------------------------------" << std::endl;
    std::cout << "Start Multicast test, " << std::endl;
    std::cout << "Please be sure to execute NetworkingUtilTests as a server on a different machine." << std::endl;
    MulticastServerDescriptor server(TEST_GROUP, TEST_PORT);
    char choice = 0;
    do {
        server.broadcast(TEST_PORT, MESSAGE, strlen(MESSAGE));
        std::cout << "A message was multicast." << std::endl;
        std::cout << R"(To resend the message press "a", if the server received it press "y")" << std::endl;
        std::cin >> choice;
        if (choice != 'a' && choice != 'y') {
            std::cout << R"(Please enter either "a" or "y" (without the apostrophes))" << std::endl;
        }
    } while (choice != 'y');

    std::cout << std::endl;
    std::cout << "------------------------------" << std::endl;
    std::cout << "Start fragmentation with multicast, please wait.... " << std::endl;

    // test compression.
    MockCompressionLibrary library;
    MessageFragmentationServer<void> fragmentationServer(FRAGMENT_SIZE, &library, &server);

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    fragmentationServer.broadcastFragmentation(TEST_PORT, TEST_MSG_TYPE, buffer, BUFFER_SIZE);

    do {
        server.broadcast(TEST_PORT, MESSAGE, strlen(MESSAGE));
        std::cout << "A message was multicast using fragments." << std::endl;
        std::cout << R"(To resend the message press "a", if the server received it press "y")" << std::endl;
        std::cin >> choice;
        if (choice != 'a' && choice != 'y') {
            std::cout << R"(Please enter either "a" or "y" (without the apostrophes))" << std::endl;
        }
    } while (choice != 'y');
}

void testMulticastServerMain() {
    std::cout << "------------------------------" << std::endl;
    std::cout << "Start Multicast test, " << std::endl;
    std::cout << "Please be sure to execute NetworkingUtilTests as a client on a different machine." << std::endl;
    MulticastServerDescriptor server(TEST_GROUP, TEST_PORT);
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    char from[16];
    std::cout << "Waiting for multicast packet..." << std::endl;
    while(!server.awaitForMessage()){
        sleep(1);
    }
    server.recv(buffer, BUFFER_SIZE, from);
    ASSERT(strcmp(buffer, MESSAGE), 0);
    std::cout << "Message was received! Please verify the message:" << std::endl;
    std::cout << "Message was sent from ip : " << from << std::endl;
    std::cout << "Is it the real ip? (y/n)" << std::endl;
    char choice = 0;
    std::cin >> choice;
    ASSERT(choice, 'y');

    std::cout << std::endl;
    std::cout << "------------------------------" << std::endl;
    std::cout << "Start fragmentation with multicast, please wait.... " << std::endl;
    // test compression.
    MockCompressionLibrary library;
    MessageFragmentationServer<void> fragmentationServer(FRAGMENT_SIZE, &library, &server);

    fragmentationServer.addHandler(TEST_MSG_TYPE, [](void *, const char *b, size_t s) {
        char zeroBuffer[BUFFER_SIZE];
        memset(zeroBuffer, 0, BUFFER_SIZE);
        ASSERT(memcmp(b, zeroBuffer, BUFFER_SIZE), 0);
        std::cout << "Message was received by the client." << std::endl;
        return true;
    });

    while (!fragmentationServer.receiveFragmentation()) {
        sleep(1);
    }
    std::cout << "Thank you for waiting, multicast test was completed." << std::endl << std::endl;
}