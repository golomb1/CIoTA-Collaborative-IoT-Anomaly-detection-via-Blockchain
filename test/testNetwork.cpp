//
// Created by master on 16/01/18.
//

#include <iostream>
#include <cstring>
#include "NetworkTest/NetworkTestMethods.h"

#define NUM_OF_ARGS 1

void printUsage(char *argv0) {
    std::string programName(argv0);
    unsigned long pos = programName.find_last_of('/');
    programName = programName.substr(pos + 1);
    std::cout << "Usage \"" << programName << ": [0/1]\"\n1 indicate Server and 0 indicate Client." << std::endl;
    exit(EXIT_FAILURE);
}

void testHelper(bool isServer) {
    std::cout << "Welcome to NetworkingUtilTests" << std::endl;
    std::cout << "NetworkingUtilTests will play as a " << (isServer ? "server" : "client") << std::endl;
    if (isServer) {
        testMulticastServerMain();
    } else {
        testMulticastClientMain();
    }
    char wait;
    do {
        std::cout << "Would you like to test the TCP Protocol package? (y/n)" << std::endl;
        std::cin >> wait;
    } while (wait != 'y' && wait != 'n');
    if (wait == 'y') {
        if (isServer) {
            testTcpProtocolServerMain();
        } else {
            testTcpProtocolClientMain();
        }
    }
    std::cout << "Finished executing successfully, exiting." << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc != 1 + NUM_OF_ARGS) {
        printUsage(argv[0]);
    }
    if (strcmp(argv[1], "server") == 0) {
        testHelper(true);
    } else if (strcmp(argv[1], "client") == 0) {
        // client
        testHelper(false);
    } else {
        std::cout << "Wrong rule." << std::endl;
        printUsage(argv[0]);
    }
    exit(EXIT_SUCCESS);
}