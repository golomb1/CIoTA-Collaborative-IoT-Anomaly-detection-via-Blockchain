//
// Created by master on 16/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_SIMPLE_TCP_PROTOCOL_H
#define CIOTA_COMPLETE_PROJECT_SIMPLE_TCP_PROTOCOL_H

/**
 *  @file    SimpleTcpProtocol.h
 *  @author  Tomer Golomb (golombt)
 *  @version 1.0
 *
 *  @brief defines headers that used both by SimpleTcpProtocolClient and SimpleTcpProtocolServer.
 *
 */

typedef struct SimpleProtocolHeaders {
    unsigned char type;
    bool isError;
    size_t len;
} SimpleProtocolHeaders;


#endif //CIOTA_COMPLETE_PROJECT_SIMPLE_TCP_PROTOCOL_H
