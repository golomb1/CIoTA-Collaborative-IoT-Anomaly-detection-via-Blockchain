//
// Created by golombt on 23/12/2017.
//

#ifndef PROJECT_BLOCKCHAIN_LISTENER_H
#define PROJECT_BLOCKCHAIN_LISTENER_H

#include <cstdio>

class BlockchainListener{
public:
    virtual void onBlockAcceptance(const char* block, size_t len) = 0;
    virtual void onPartialBlockAcceptance(const char* block, size_t len) = 0;
    virtual void onPartialBlockAcceptance(const char *block, size_t len, bool selfUpdate) = 0;
    virtual void setCompleteBlockSize(size_t completeBlockSize) = 0;
};
#endif //PROJECT_BLOCKCHAIN_LISTENER_H
