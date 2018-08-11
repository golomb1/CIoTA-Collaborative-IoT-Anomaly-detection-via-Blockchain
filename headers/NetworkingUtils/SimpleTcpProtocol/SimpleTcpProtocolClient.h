//
// Created by master on 16/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_SIMPLE_TCP_H
#define CIOTA_COMPLETE_PROJECT_SIMPLE_TCP_H

#include "../SimpleProtocol.h"
#include "SimpleTcpProtocol.h"

/**
 *
 */
class SimpleTcpProtocolClient {
public:
    /**
     * Constructor - create a new instance of SimpleTcpProtocolClient.
     */
    SimpleTcpProtocolClient();

    ~SimpleTcpProtocolClient();

    /**
     * @return true if the session is open.
     */
    bool isSessionOpen();

    /**
     * Open new session with a server at the given ip and port.
     * @param ip of the server.
     * @param port of the server.
     */
    void startSession(const char *ip, unsigned short port);

    /**
     * Close the current session, if no session is opened, nothing occurs.
     */
    void endSession();

    /**
     * Send packet in the open session.
     * Can be preformed only if the session is opened.
     * Otherwise InvalidStateException is thrown.
     * @param type of the message.
     * @param buffer to send.
     * @param len of the buffer.
     * @param outLen is the return value length.
     * @return a buffer containing the response from the server.
     */
    char *send(unsigned char type, const void *buffer, size_t len, size_t *outLen);

protected:
    int _socket;
};


#endif //CIOTA_COMPLETE_PROJECT_SIMPLE_TCP_H
