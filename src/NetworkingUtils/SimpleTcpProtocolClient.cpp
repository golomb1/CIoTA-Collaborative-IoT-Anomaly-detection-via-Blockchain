//
// Created by master on 16/01/18.
//

#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include "../../headers/NetworkingUtils/SimpleTcpProtocol/SimpleTcpProtocolClient.h"
#include "../../headers/CppUtils/Exceptions.h"
#include "../../headers/NetworkingUtils/SimpleTcpProtocol/ProtocolException.h"
#include <iostream>

SimpleTcpProtocolClient::SimpleTcpProtocolClient() : _socket(-1) {

}

SimpleTcpProtocolClient::~SimpleTcpProtocolClient() {
    endSession();
}


bool SimpleTcpProtocolClient::isSessionOpen() {
    return _socket >= 0;
}

void SimpleTcpProtocolClient::startSession(const char *ip, unsigned short port) {
    if (isSessionOpen()) {
        throw InvalidStateException("SimpleTcpProtocol:startSession", "session is already open.");
    }
    else {
        if ((_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            throw SystemError("SimpleTcpProtocol:startSession:socket failed");
        }
        sockaddr_in serverAddress{};
        memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        serverAddress.sin_addr.s_addr = inet_addr(ip);
        if (connect(_socket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
            close(_socket);
            _socket = -1;
            throw SystemError("SimpleTcpProtocol:startSession:connect failed");
        }
    }
}

void SimpleTcpProtocolClient::endSession() {
    if (isSessionOpen()) {
        close(_socket);
        _socket = -1;
    }
}

char *SimpleTcpProtocolClient::send(unsigned char type, const void *buffer, size_t len, size_t *outLen) {
    SimpleProtocolHeaders headers{};
    memset(&headers, 0, sizeof(headers));
    headers.type = type;
    headers.isError = false;
    headers.len = len;
    if (::send(_socket, &headers, sizeof(headers), 0) != sizeof(headers)) {
        throw SystemError("SimpleTcpProtocol:SimpleTcpProtocol:send headers failed");
    }
    if(len > 0) {
        if (::send(_socket, buffer, len, 0) != len) {
            throw SystemError("SimpleTcpProtocol:SimpleTcpProtocol:send body failed");
        }
    }
    if (::recv(_socket, &headers, sizeof(headers), 0) != sizeof(headers)) {
        throw SystemError("SimpleTcpProtocol:SimpleTcpProtocol:recv headers failed");
    }
    char *out = nullptr;
    if(headers.len > 0) {
        out = new char[headers.len];
        if (::recv(_socket, out, headers.len, 0) != headers.len) {
            delete[] out;
            throw SystemError("SimpleTcpProtocol:SimpleTcpProtocol:recv body failed");
        }
    }
    *outLen = headers.len;
    if(headers.isError){
        if(out == nullptr){
            throw ProtocolException("");
        }
        else{
            ProtocolException e(out);
            delete[] out;
            throw e;
        }
    }
    else {
        return out;
    }
}

