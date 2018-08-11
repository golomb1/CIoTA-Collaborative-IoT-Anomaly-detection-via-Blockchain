//
// Created by tomer on 20/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_PROTOCOL_EXCEPTION_H
#define CIOTA_COMPLETE_PROJECT_PROTOCOL_EXCEPTION_H

#include "../../CppUtils/Exceptions.h"

/**
 *  @file    ProtocolException.h
 *  @author  Tomer Golomb (golombt)
 *  @version 1.0
 *
 *  @brief defines an class to indicate that execution of a protocol failed.
 *
 */


class ProtocolException : public MessageException{
public:
    explicit ProtocolException(const char *msg) : MessageException("%s", msg) {}

};

#endif //CIOTA_COMPLETE_PROJECT_PROTOCOL_EXCEPTION_H
