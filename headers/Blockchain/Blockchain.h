//
// Created by golombt on 17/12/2017.
//

#ifndef BLOCKCHAIN_BLOCKCHAIN_H
#define BLOCKCHAIN_BLOCKCHAIN_H


#include <atomic>
#include <chrono>
#include <thread>
#include <mutex>
#include <ostream>
#include "Block.h"
#include "BlockchainApplication.h"
#include "../CppUtils/SerializableBoundedQueue.h"
#include "../CppUtils/Logger.h"


#define VIOLATION_OLD_CHAIN                (-1)
#define VIOLATION_TOO_MANY_PARTIAL_BLOCKS  (-2)
#define VIOLATION_WRONG_HASH               (-3)
#define VIOLATION_FAR_CHAIN                (-4)
#define VIOLATION_WRONG_INDEX              (-5)
#define VIOLATION_FAR_CHAIN_COLLISION      (-6)
#define VIOLATION_UNVALID_BLOCK            (-7)



template <unsigned short H>
class Blockchain {

public:
    explicit Blockchain(size_t capacity, BlockchainApplication<H>* application) :
            _partialBlock{}, _chain(capacity), _application(application) {   }


    /**
     * @brief Receive a new chain and update the current chain with the given chain.
     * @param chain received from a different node.
     * @return how many blocks where accepted from the new chain.
     */
    int receiveChain(const SerializableBoundedQueue* chain) {
        LOG_TRACE_5("Blockchain receiveChain with {0:d}", chain->size())
        std::lock_guard<std::mutex> lock(_chainMutex);
        // we might have part of this chain,
        // so search from which index the chain includes new data.
        size_t newChainStartIndex = 1;
        // keep the previous block of hash integrity.
        const char *prev = chain->tail();
        size_t prevLength = chain->tailLength();
        bool shouldAcceptPrev = true;
        // if our chain is empty it doesn't matter.
        if(_chain.size() > 0) {
            size_t index;
            int res = detectCollision(chain, &index);
            LOG_TRACE_5("Blockchain receiveChain detect collision {0:d}", res)
            if(res!=0){
                if(!((res == VIOLATION_FAR_CHAIN || res == VIOLATION_FAR_CHAIN_COLLISION) &&
                        _application->acceptFarChain))
                {
                    return res;
                }
            }
            else{
                prev = chain->at(index - 1);
                prevLength = chain->lengthAt(index - 1);
                shouldAcceptPrev = false;
                newChainStartIndex = index;
            }
        }
        LOG_TRACE_5("Blockchain receiveChain start validating block")
        // start checking the list of chain for its integrity.
        for (size_t i = newChainStartIndex; i < chain->size(); i++) {
            // if it not the last block, then it should not be partial block.
            if (!_application->isCompletedBlock(prev, prevLength)) {
                LOG_TRACE_4("Blockchain receiveChain Too many incomplete blocks")
                return VIOLATION_TOO_MANY_PARTIAL_BLOCKS; // BLOCK_VIOLATION: contains too many incomplete block.
            }
            // hash and index integrity check should be preformed only when prev != current.
            // check hash integrity and index integrity.
            char prevHash[H];
            _application->getHash(prevHash, prev, prevLength);
            /*if (memcmp(BLOCK_PREV_HASH(chain->at(i)), prevHash, H) != 0) {
                LOG_TRACE("Blockchain receiveChain Wrong hash")
                return VIOLATION_WRONG_HASH; // BLOCK_VIOLATION: prev hash inconsistency.
            }*/
            if(BLOCK_ID(chain->at(i)) != BLOCK_ID(prev) + 1){
                LOG_TRACE_5("Blockchain receiveChain Wrong index {0} {1}", BLOCK_ID(prev), BLOCK_ID(chain->at(i)))
                return VIOLATION_WRONG_INDEX; // BLOCK_VIOLATION: index inconsistency.
            }
            prev = chain->at(i);
            prevLength = chain->lengthAt(i);
        }
        LOG_TRACE_5("Blockchain receiveChain chain is valid")
        // the chain
        int accepted = 0;
        // should accept previous block? if yes then start accepting from the previous block.
        if(shouldAcceptPrev){
            newChainStartIndex--;
        }
        // the chain is verified, start to accept the blocks.
        for (size_t i = newChainStartIndex; i < chain->size() - 1; i++) {
            if(acceptNewBlock(chain->at(i), chain->lengthAt(i)) >= 0){
                LOG_TRACE_5("Blockchain receiveChain accepted block")
                accepted++;
            }
            else{
                LOG_TRACE_5("Blockchain receiveChain block rejected")
                return accepted;
            }
        }

        // check the last block
        const char *partialBlock = chain->at(chain->size() - 1);
        size_t partialBlockLength = chain->lengthAt(chain->size() - 1);
        if (_application->isCompletedBlock(partialBlock, partialBlockLength)) {
            LOG_TRACE_5("Blockchain receiveChain partial is complete")
            if(acceptNewBlock(partialBlock, partialBlockLength) >= 0){
                LOG_TRACE_5("Blockchain receiveChain partial is complete and accepted")
                accepted++;
            }
            else{
                LOG_TRACE_5("Blockchain receiveChain partial is complete and rejected")
            }
            return accepted;
        } else {
            LOG_TRACE_5("Blockchain receiveChain partial len {0:d}", BLOCK_NUM_OF_RECORDS(partialBlock,H))
            // should replace my partial block?
            if (shouldAcceptPartial(partialBlock))
            {
                if (BLOCK_NUM_OF_RECORDS(partialBlock,H) > 0 && _application->testPartialBlock(partialBlock, partialBlockLength)) {
                    LOG_TRACE_5("Blockchain receiveChain partial pass the test")
                    _partialBlock.clear();
                    _partialBlock.append(partialBlock, partialBlockLength);
                    _application->notifyOnPartialBlockAcceptance(&_partialBlock);
                }
                else{
                    LOG_TRACE_5("Blockchain receiveChain partial didn't pass the test")
                }
            }
        }
        return accepted;
    }


    bool shouldAcceptPartial(const char *partialBlock) const {
        if(_partialBlock.empty()){
            return true;
        } else {
            bool alreadyInTheReceived = hasRecordFrom<H>(partialBlock, _application->getFrom(),
                                                      _application->getFromSize());
            bool alreadyInTheCorrect = hasRecordFrom<H>(&_partialBlock, _application->getFrom(),
                                                     _application->getFromSize());
            size_t numOfBlocksInReceivedPB = BLOCK_NUM_OF_RECORDS(partialBlock, H);
            size_t numOfBlocksInCorrectPB = BLOCK_NUM_OF_RECORDS(_partialBlock.data(), H);
            if (!alreadyInTheReceived) {
                numOfBlocksInReceivedPB++;
            }
            if (!alreadyInTheCorrect) {
                numOfBlocksInCorrectPB++;
            }
            return numOfBlocksInReceivedPB > numOfBlocksInCorrectPB;
        }
    }


    /**
     * @return the partial block of this chain.
     * @attention do not free this pointer.
     */
    MemBuffer* getPartialBlock(){
        return &_partialBlock;
    }


    MemBuffer* updatePartialBlock(){
        std::lock_guard<std::mutex> lock(_chainMutex);
        char previousHash[H];
        memset(previousHash, 0, H);
        size_t id = 0;
        if(_chain.size() != 0){
            _application->getHash(previousHash, _chain.head(), _chain.headLength());
            id = BLOCK_ID(_chain.head()) + 1;
        }
        if(!_partialBlock.empty()){
            setBlockPrevHash<H>(&_partialBlock,previousHash);
            setBlockId<H>(&_partialBlock, id);
            if(!hasRecordFrom<H>(&_partialBlock, _application->getFrom(), _application->getFromSize())){
                MemBuffer record;
                _application->makeRecord(_partialBlock.data(), _partialBlock.size(), &record);
                addRecordContent<H>(&_partialBlock, record.data(), record.size());
            }
        }
        else{
            MemBuffer seed;
            MemBuffer record;
            time_t timestamp = time(nullptr);
            _application->getSeed(id, timestamp, previousHash, &seed);
            createNewBlock<H>(&_partialBlock, id, timestamp, previousHash, seed.data(), seed.size());
            _application->makeRecord(_partialBlock.data(), _partialBlock.size(), &record);
            addRecordContent<H>(&_partialBlock, record.data(), record.size());
        }
        size_t t = BLOCK_NUM_OF_RECORDS(_partialBlock.data(), H);
        _application->notifyOnPartialBlockAcceptance(&_partialBlock, false);
        return &_partialBlock;
    }


    /**
     * @brief Perform operation on the chain.
     * @param consumer is a function that will accept the chain value.
     * @attention This method append the partial block to the chain and send it to the consumer.
     * After the consumer finished, the partial block is removed from the buffer.
     * So the consumer should deep copy the chain if it keeps it since the chain will be changed.
     */
    void consumeChain(std::function<void(SerializableBoundedQueue*)> consumer){
        std::lock_guard<std::mutex> lock(_chainMutex);
        if(_chain.size() != 0 && !_partialBlock.empty()){
            char previousHash[H];
            _application->getHash(previousHash, _chain.head(), _chain.headLength());
            size_t id = BLOCK_ID(_chain.head()) + 1;
            setBlockPrevHash<H>(&_partialBlock,previousHash);
            setBlockId<H>(&_partialBlock, id);
        }
        if(_partialBlock.empty()) {
            //consumer(&_chain);
        } else {
            _chain.push_back(_partialBlock.data(), _partialBlock.size());
            for(int i=0; i < _chain.size(); i++){
                LOG_TRACE_4("Broadcasted chain {0}", BLOCK_ID(_chain.at(i)))
            }
            consumer(&_chain);
            _chain.removeHead();
        }
    }



private:
    std::mutex        _chainMutex;
    MemBuffer         _partialBlock{};
    SerializableBoundedQueue _chain;
    BlockchainApplication<H>* _application;


protected:

    /**
     * @brief helper function that handle the acceptance of a new block.
     * If the block is equal to the partial block than throw away the partial block.
     * @param block to accept.
     * @param blockLength is the length of the block.
     * @return 1 if the block could be added or negative value otherwise.
     */
    virtual int acceptNewBlock(const char* block, size_t blockLength){
        if (!_application->isBlockAcceptable(block, blockLength)) {
            return VIOLATION_UNVALID_BLOCK; // block content inconsistency.
        }
        // check if it is our partial block, and it does then clear the partial block
        if(!_partialBlock.empty() && BLOCK_NUM_OF_RECORDS(block,H) > 0) {
            const char *blockSeed = BLOCK_SEED(block, H);
            size_t blockSeedSize = BLOCK_SEED_SIZE(block, H);
            const char *partialSeed = BLOCK_SEED(_partialBlock.data(), H);
            size_t partialSeedSize = BLOCK_SEED_SIZE(_partialBlock.data(), H);
            if (blockSeedSize == partialSeedSize && memcmp(blockSeed, partialSeed, blockSeedSize) == 0) {
                _partialBlock.clear();
            }
        }
        _application->notifyOnBlockAcceptance(block, blockLength);
        if (blockLength < _chain.capacity()) {
            _chain.push_back(block, blockLength);
        } else {
            if (BLOCK_META_DATA_SIZE(block, H) > _chain.capacity()) {
                //_chain.push_back(block, BLOCK_META_DATA_SIZE(block, H));
            } else {
                // if still bigger than capacity, act according to the policy.
                // TODO find better implementation for this.
                //_chain.push_back(block, BLOCK_META_DATA_SIZE(block, H));
            }
        }
        return 1;
    }

    /**
     * @brief find a point where the given chain collide with this class chain
     * (the newest block that is shared by both of them).
     * @param chain1
     * @param chain2
     * @param sharedBlockIndex is the output of the block index that they both agree on.
     * @return 0 if success or negative value otherwise.
     */
    int detectCollision(const SerializableBoundedQueue* chain1, size_t* sharedBlockIndex) const {
	for(int i=0; i < chain1->size(); i++){
           LOG_TRACE_4("Got chain {0}", BLOCK_ID(chain1->at(i)))
        }
        if(!_partialBlock.empty() && BLOCK_ID(_partialBlock.data()) > BLOCK_ID(chain1->head())){
                return VIOLATION_OLD_CHAIN;
        }
        if(BLOCK_ID(chain1->head()) <= BLOCK_ID(_chain.head())) {
                // chain1 is shorter then chain2
                return VIOLATION_OLD_CHAIN;
        }
        else{
            // chain1 longer then chain2.
            // check if too far.
            if(BLOCK_ID(chain1->tail()) > BLOCK_ID(_chain.head())) {
                // to far
                return VIOLATION_FAR_CHAIN;
            }
            else{
                // there collision, search for it.
                // if no shared block can be found then chain 1 is considered too far (collision).
                size_t chain1Index;
                size_t chain2Index;
                if(BLOCK_ID(chain1->tail()) > BLOCK_ID(_chain.tail())){
                    // chain1 tail is the start
                    chain1Index = 0;
                    chain2Index = BLOCK_ID(chain1->tail()) - BLOCK_ID(_chain.tail());
                }
                else{
                    // chain2 tail is the start
                    chain1Index = BLOCK_ID(_chain.tail()) - BLOCK_ID(chain1->tail());
                    chain2Index = 0;
                }
                // start searching
                size_t sharedBlocks = 0;
                for(; chain1Index < chain1->size() && chain2Index < _chain.size(); chain1Index++, chain2Index++){
                    const char* chain1Block = chain1->at(chain1Index);
                    size_t chain1BlockLen = chain1->lengthAt(chain1Index);
                    const char* chain2Block = _chain.at(chain2Index);
                    size_t chain2BlockLen = _chain.lengthAt(chain2Index);
                    char chain1Hash[H];
                    char chain2Hash[H];
                    _application->getHash(chain1Hash, chain1Block, chain1BlockLen);
                    _application->getHash(chain2Hash, chain2Block, chain2BlockLen);
                    if(BLOCK_ID(chain1->at(chain1Index)) == BLOCK_ID(_chain.at(chain2Index)) || memcmp(chain1Hash, chain2Hash, H) == 0){
                        sharedBlocks++;
                    }
                    else{
                        // found
                        break;
                    }
                }
                if(sharedBlocks == 0){
                    // no connection was founded.
                    return VIOLATION_FAR_CHAIN_COLLISION;
                }
                else{
                    *sharedBlockIndex = chain1Index;
		    LOG_TRACE_4("Shared Index {0}", *sharedBlockIndex);
                    return 0;
                }
            }
        }
    }
};



#endif //BLOCKCHAIN_BLOCKCHAIN_H
