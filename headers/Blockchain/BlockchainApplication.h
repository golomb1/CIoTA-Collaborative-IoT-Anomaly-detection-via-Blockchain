//
// Created by golombt on 17/12/2017.
//

#ifndef BLOCKCHAIN_BLOCKCHAIN_APPLICATION_H
#define BLOCKCHAIN_BLOCKCHAIN_APPLICATION_H

#include "Block.h"


/**
 *  @file    BlockchainApplication.h
 *  @author  Tomer Golomb (golombt)
 *  @version 1.0
 *
 *  @brief defines an interface for BlockchainApplication class that uses an hash function with H as the its output size.
 *  BlockchainApplication is used to configure the blockchain properties.
 *  @tparam H - is the output size (in bytes) of the hash function.
 */
template <unsigned short H>
class BlockchainApplication{
protected:
    const MemBuffer* _id;

public:
    const bool acceptFarChain;

    /**
     * Constructor
     * @param acceptFarChain flag that indicate whether to accept chains without their origin.
     */
    explicit BlockchainApplication(const MemBuffer* id, bool acceptFarChain)
            : _id(id), acceptFarChain(acceptFarChain){}

    /**
     * Function to determine if a block is closed, or still in process.
     * @param block to check.
     * @param len is the length of pBlock.
     * @return true if the block is completed or false if it partial.
     */
    virtual bool isCompletedBlock(const char* block, size_t len) = 0;

    /**
     * Calculate an hash string of length H for the given block and store the result in out.
     * @param out is the out buffer of size H.
     * @param pBlock is the given block.
     * @param len is the length of pBlock.
     */
    virtual void getHash(char out[H], const char *pBlock, size_t len) = 0;

    /**
     * Output seed for a new partial block, wil be used to distingue partial blocks apart.
     * @param blockId is the id of the new block (the length of the chain).
     * @param timestamp to be used inside the block.
     * @param prevBlockHash is the hash of the previous block.
     * @param out is the output, the seed should be stored in this buffer.
     */
    virtual void getSeed(size_t blockId, time_t timestamp, const char prevBlockHash[H],MemBuffer* out) = 0;


    /**
     * Function to determine if a block is acceptable, or inconsistencies with the rest of the chain.
     * @param pBlock the new block to be appended in the chain.
     * @param len is the length of pBlock.
     * @return true if the block can be appended into the chain, and false otherwise.
     */
    virtual bool isBlockAcceptable(const char *pBlock, size_t len) = 0;

    /**
     * This function should test the partial block to determine if it legal,
     * or inconsistencies with the rest of the chain.
     * @param pBlock is the partial block to be tested.
     * @param len is the length of pBlock.
     * @return true if the partial block is acceptable and false otherwise.
     */
    virtual bool testPartialBlock(const char *pBlock, size_t len) = 0;

    /**
     * Notify on acceptance of new block into the chain.
     * @param pBlock is the block that was appended into the chain.
     * @param len  is the length of pBlock.
     */
    virtual void notifyOnBlockAcceptance(const char *pBlock, size_t len) = 0;

    /**
     * Notify on acceptance of new partial block.
     * @param partialBlock is the partial block that was accepted.
     */
    virtual void notifyOnPartialBlockAcceptance(MemBuffer* partialBlock) = 0;

    /**
     * Notify on acceptance of new partial block.
     * @param partialBlock is the partial block that was accepted.
     * @param selfUpdate is a flag that indicate whether the agent changed the partial
     * block himself or accepted a new one.
     */
    virtual void notifyOnPartialBlockAcceptance(MemBuffer *partialBlock, bool selfUpdate) = 0;


    /**
     * Generate new record for the partial block.
     * @param block is the partial block that is generated.
     * @param len is the length of block
     * @param out is the output buffer, the record should be stored there.
     */
    virtual void makeRecord(const char *block, size_t len, MemBuffer *out) = 0;

    /**
     * @return the id of the application instance, used for signing and distingue participants.
     * @attention Do not free this pointer.
     */
    virtual const char* getFrom() const{
        return _id->data();
    }

    /**
     * @return the length of this application instance's id, used for signing and distingue participants.
     */
    virtual size_t getFromSize() const{
        return _id->size();
    };

    /**
     *
     * @param id copy this instance of BlockchainApplication and change it's id.
     * @return the copy pointer.
     */
    virtual BlockchainApplication<H>* copy(MemBuffer* id) = 0;
};


#endif //BLOCKCHAIN_BLOCKCHAIN_APPLICATION_H
