//
// Created by master on 16/01/18.
//

#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include "../../headers/NetworkingUtils/MulticastServerDescriptor.h"
#include "../../headers/CppUtils/Exceptions.h"
#include <iostream>


MulticastServerDescriptor::MulticastServerDescriptor(std::string &group, unsigned short port) :
        MulticastServerDescriptor(group.c_str(), port, true) {}


MulticastServerDescriptor::MulticastServerDescriptor(const char *group, unsigned short port) :
        MulticastServerDescriptor(group, port, true) {}


MulticastServerDescriptor::MulticastServerDescriptor(const char *group, unsigned short port, bool openSocket) :
        _udpSockFd(-1), _group(group), _port(port) {
    if (openSocket) {
        open();
    }
}


int MulticastServerDescriptor::recv(char *buffer, size_t len, char *fromAddress) {
    bool self = false;
    char *ip = nullptr;
    ssize_t nBytes = 0;

    sockaddr_in selfAddress{};
    socklen_t selfAddressLen = sizeof(selfAddress);
    getsockname(_udpSockFd, (sockaddr *) (&selfAddress), &selfAddressLen);

    do {
        sockaddr_in address{};
        socklen_t addressLen = sizeof(address);
        nBytes = recvfrom(_udpSockFd, buffer, len, 0, (sockaddr *) (&address), &addressLen);
        ip = inet_ntoa(address.sin_addr);
        self = true;//address.sin_addr.s_addr == selfAddress.sin_addr.s_addr;
    } while (!self);
    strcpy(fromAddress, ip);
    return static_cast<int>(nBytes);
}


int MulticastServerDescriptor::broadcast(unsigned short port, const char *buffer, size_t len) {
    sockaddr_in address{};
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(_group.c_str());
    address.sin_port = htons(port);
    ssize_t nBytes = sendto(_udpSockFd, buffer, len, 0, reinterpret_cast<const sockaddr *>(&address), sizeof(address));
    return static_cast<int>(nBytes);
}


bool MulticastServerDescriptor::awaitForMessage() {
    int count;
    ioctl(_udpSockFd, FIONREAD, &count);
    return count > 0;
}


bool MulticastServerDescriptor::open() {
    try {
        uint yes = 1;

        sockaddr_in address{};
        ip_mreq mReq{};

        if ((_udpSockFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            throw SystemError("MulticastServerDescriptor - socket failed");
        }
        if (setsockopt(_udpSockFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
            throw SystemError("MulticastServerDescriptor - setsockopt SO_REUSEADDR failed");
        }

        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = htons(_port);

        if (bind(_udpSockFd, reinterpret_cast<const sockaddr *>(&address), sizeof(address)) < 0) {
            throw SystemError("MulticastServerDescriptor - bind failed");
        }

        memset(&mReq, 0, sizeof(mReq));
        mReq.imr_multiaddr.s_addr = inet_addr(_group.c_str());
        mReq.imr_interface.s_addr = htonl(INADDR_ANY);
        if (setsockopt(_udpSockFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mReq, sizeof(mReq)) < 0) {
            throw SystemError("MulticastServerDescriptor - setsockopt IP_ADD_MEMBERSHIP failed");
        }
        u_char loop = 0;
        if (setsockopt(_udpSockFd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0) {
            throw SystemError("MulticastServerDescriptor - setsockopt IP_ADD_MEMBERSHIP failed");
        }

        return true;
    }
    catch (SystemError &e) {
        close();
        __throw_exception_again e;
    }
}

bool MulticastServerDescriptor::close() {
    if (isOpen()) {
        ::close(_udpSockFd);
        _udpSockFd = -1;
        return true;
    }
    return false;
}

MulticastServerDescriptor::~MulticastServerDescriptor() {
    close();
}

bool MulticastServerDescriptor::isOpen() {
    return _udpSockFd >= 0;
}
