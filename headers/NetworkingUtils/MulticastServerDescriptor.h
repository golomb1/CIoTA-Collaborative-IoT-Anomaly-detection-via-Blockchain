//
// Created by master on 16/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_MULTICAST_SERVER_DESCRIPTOR_H
#define CIOTA_COMPLETE_PROJECT_MULTICAST_SERVER_DESCRIPTOR_H

#include <string>
#include "BroadcastServer.h"

typedef int SOCKET;


class MulticastServerDescriptor : public BroadcastServer {

public:
    /**
     * Constructor for MulticastServerDescriptor and open it.
     * @param group of the multicast group
     * @param port of the multicast group
     */
    MulticastServerDescriptor(std::string &group, unsigned short port);

    /**
     * Constructor for MulticastServerDescriptor and open it.
     * @param group of the multicast group
     * @param port of the multicast group
     */
    MulticastServerDescriptor(const char *group, unsigned short port);

    /**
     * Constructor for MulticastServerDescriptor and open it.
     * @param group of the multicast group
     * @param port of the multicast group
     * @param open is a flag that indicate whether to open this server or not yet.
     */
    MulticastServerDescriptor(const char *group, unsigned short port, bool open);

    /**
     * Destructor that close this socket.
     */
    virtual ~MulticastServerDescriptor();

    /**
     * Receive broadcast.
     * @param buffer will store the broadcast message.
     * @param len is the length of buffer (max reading).
     * @param fromAddress will store the ip of the sender.
     * @return the number of bytes read (stored in buffer) or negative as error (use perror)
     */
    int recv(char *buffer, size_t len, char *fromAddress) override;

    /**
     * Send broadcast to the multicast group in the given port.
     * @param port to send the broadcast.
     * @param buffer to send.
     * @param len is the length of buffer.
     * @return the number of bytes sent or negative as error (use perror).
     */
    int broadcast(unsigned short port, const char *buffer, size_t len) override;

    /**
     * @return true if there is a message in the socket, and false otherwise.
     */
    bool awaitForMessage() override;

    /**
     * Open this socket.
     * @return true if success or exception otherwise.
     */
    bool open();


    /**
     * Close this socket, if already closed nothing occurs.
     * @return true the socket closed and false otherwise.
     */
    bool close();

    /**
     * @return true if this socket is open
     */
    bool isOpen();

private:
    SOCKET _udpSockFd;
    std::string _group;
    unsigned short _port;
};


#endif //CIOTA_COMPLETE_PROJECT_MULTICAST_SERVER_DESCRIPTOR_H
