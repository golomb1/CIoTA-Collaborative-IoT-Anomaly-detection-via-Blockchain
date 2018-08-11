//
// Created by master on 16/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_SIMPLE_PROTOCOL_H
#define CIOTA_COMPLETE_PROJECT_SIMPLE_PROTOCOL_H

#include <cstdio>

class SimpleProtocol {
public:
    virtual bool isSessionOpen()=0;

    virtual void startSession(const char *ip, unsigned short port)=0;

    virtual void endSession()=0;

    virtual char *send(unsigned char type, const void *buffer, size_t len, size_t *outLen)=0;
};


#endif //CIOTA_COMPLETE_PROJECT_SIMPLE_PROTOCOL_H
