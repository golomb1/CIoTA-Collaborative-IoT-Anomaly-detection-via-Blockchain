//
// Created by golombt on 20/12/2017.
//

#ifndef NETWORKING_UTILS_FILE_DESCRIPTOR_WRAPPER_H
#define NETWORKING_UTILS_FILE_DESCRIPTOR_WRAPPER_H


#include <cstdio>

#define IP_ADDRESS_STRING_LENGTH 16

class BroadcastServer {
public:
    virtual int recv(char* buffer, size_t len, char fromAddress[IP_ADDRESS_STRING_LENGTH]) = 0;
    virtual int broadcast(unsigned short port, const char* buffer, size_t len) = 0;
    virtual bool awaitForMessage() = 0;
};


#endif //NETWORKING_UTILS_FILE_DESCRIPTOR_WRAPPER_H
