//
// Created by master on 08/02/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_PIBRELLA_BLOCKCHAIN_LISTENER_H
#define CIOTA_COMPLETE_PROJECT_PIBRELLA_BLOCKCHAIN_LISTENER_H


#include <headers/Blockchain/BlockchainListener.h>

class PibrellaBlockchainListener : public BlockchainListener {
private:
    int _fd;

public:

    PibrellaBlockchainListener();

    ~PibrellaBlockchainListener();

    void onBlockAcceptance(const char *block, size_t len) override;

    void onPartialBlockAcceptance(const char *block, size_t len) override;

    void onPartialBlockAcceptance(const char *block, size_t len, bool selfUpdate) override;

    void finishTraining();

    void setCompleteBlockSize(size_t completeBlockSize) override;
};


#endif //CIOTA_COMPLETE_PROJECT_PIBRELLA_BLOCKCHAIN_LISTENER_H
