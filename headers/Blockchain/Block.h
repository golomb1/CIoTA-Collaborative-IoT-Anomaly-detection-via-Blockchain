//
// Created by golombt on 17/12/2017.
//

#ifndef BLOCKCHAIN_BLOCK_H
#define BLOCKCHAIN_BLOCK_H


#include <utility>
#include <map>
#include <cstring>
#include "BlockRecord.h"
#include "../CppUtils/Exceptions.h"
#include "../CppUtils/Cursor.h"



/**
 *  @file    Block.h
 *  @author  Tomer Golomb (golombt)
 *  @version 1.0
 *
 *  @brief defines an interface for Block class that uses an hash function with H as the its output size.
 *  @tparam H is the output size (in bytes) of the hash function used.
 *  Block is a buffer that represents a blockchain block.
 *  Each block is composed from the following fields (in that order):
 *
 *  Field | type | setter | getter | Description
 *  ----- | ---- | ------ | ------ | -----------
 *  Block id | size_t | setBlockId | BLOCK_ID | The block position inside the blockchain.
 *  Timestamps | time_t | None | BLOCK_TIMESTAMP | The block's creation time.
 *  Previous Hash | char[H] | setBlockPrevHash | BLOCK_PREV_HASH | The hash of the previous block in tha chain.
 *  Block Seed Size | size_t | None | BLOCK_SEED_SIZE | The size of the seed buffer.
 *  Block Seed | char* | None | BLOCK_SEED | The seed of this block.
 *  Number Of Records | size_t | addRecordContent | BLOCK_NUM_OF_RECORDS | The number of records inside this block.
 *  Block data | char* | None | BLOCK_DATA, getBlockRecord | the list of records.
 *
 *
 *  Additional utilities functions:
 *  * createNewBlock - create a new block structure.
 *  * blockRecordsBegin - return an iterator on the block content.
 *  * blockRecordsEnd - return an end iterator on the block content (when to stop).
 *  * hasRecordFrom - return whether a block contains a record from a given participant.
 */



/* Defines getters for block content */
#define BLOCK_ID(block)                            (*((size_t*)(block)))
#define BLOCK_TIMESTAMP(block)                     (*((time_t*)((block) + sizeof(size_t))))
#define BLOCK_PREV_HASH(block)                     ((block) + sizeof(size_t) + sizeof(time_t))
#define BLOCK_SEED_SIZE(block, H)                  (*((size_t*)(BLOCK_PREV_HASH(block) + (H))))
#define BLOCK_SEED(block,H)                        (BLOCK_PREV_HASH(block) + (H) + sizeof(size_t))
#define BLOCK_NUM_OF_RECORDS_POINTER(block, H)     ((size_t*)(BLOCK_SEED(block, H) + BLOCK_SEED_SIZE(block, H)))
#define BLOCK_NUM_OF_RECORDS(block, H)             (*(BLOCK_NUM_OF_RECORDS_POINTER(block, H)))
#define BLOCK_DATA(block, H)                       (BLOCK_SEED(block, H) + BLOCK_SEED_SIZE(block, H) + sizeof(size_t))
#define BLOCK_META_DATA_SIZE(block, H)             ((block) - BLOCK_DATA(block,H))






/**
 * Block record iterator.
 * A forward iterator that can be used to iterate on a block records.
 */
class BlockRecordIterator {
private:
    const char* _content;
    size_t _numOfRecords;

    void next(){
        auto * record = (BlockRecordHeader*)_content;
        _content = (((char*)record) + RECORD_LENGTH(((char*)record)));
    }

public:
    typedef BlockRecordIterator self_type;
    typedef const char*& reference;
    typedef const char**  pointer;

    BlockRecordIterator(size_t numOfRecords, const char *content) : _content(content), _numOfRecords(numOfRecords) {}

    self_type operator++(){
        self_type i=*this;
        next();
        return i;
    }
    self_type operator++(int junk) { next(); return *this; }
    reference operator*() { return _content; }
    pointer operator->() { return &_content; }
    bool operator==(const self_type& rhs) { return _content == rhs._content; }
    bool operator!=(const self_type& rhs) { return _content != rhs._content; }

    size_t size() const {

    }

    void end(){
        for(size_t i=0; i < _numOfRecords; i++){
            next();
        }
    }
};


/**
 * @brief Create a new block structure.
 * @tparam H is the output size of the hash function.
 * @param block is the buffer to write the block content into.
 * @param blockId is the id of the block (the position of the block in the chain).
 * @param timestamp of the block creation.
 * @param prevBlockHash the hash of the previous block in the chain.
 * @param seed for the block
 * @param seedLen the length of the seed.
 * @return a pointer to the block parameter.
 */
template <unsigned short H>
MemBuffer* createNewBlock(MemBuffer* block,
                          size_t blockId,
                          time_t timestamp,
                          const char prevBlockHash[H],
                          const char* seed,
                          size_t seedLen)
{
    block->clear();
    block->reserveMore(sizeof(size_t) + sizeof(time_t) + H + sizeof(size_t) + seedLen + sizeof(size_t));
    block->append(&blockId, sizeof(size_t));
    block->append(&timestamp, sizeof(time_t));
    if(prevBlockHash != nullptr) {
        block->append(prevBlockHash, H);
    }
    else{
        block->appendValue(0,H);
    }
    block->append(&seedLen, sizeof(size_t));
    block->append(seed, seedLen);
    block->appendValue(0,sizeof(size_t));
    return block;
}


/**
 * @brief set the blockId.
 * @tparam H is the output size of the hash function.
 * @param block to change.
 * @param id is the new block id.
 */
template <unsigned short H>
void setBlockId(MemBuffer *block, size_t id){
    memcpy(block->data(), &id, sizeof(size_t));
}

/**
 * @brief set the value of the previous hash.
 * @tparam H is the output size of the hash function.
 * @param block to change.
 * @param hash is the value to write inside the "previous hash" field.
 */
template <unsigned short H>
void setBlockPrevHash(MemBuffer* block, const char hash[H]){
    memcpy(BLOCK_PREV_HASH(block->data()), hash, H);
}

/**
 * @brief Return an iterator for the given block content.
 * @tparam H is the output size of the hash function.
 * @param block to iterate.
 * @return the iterator.
 */
template <unsigned short H>
BlockRecordIterator blockRecordsBegin(const char* block){
    return {BLOCK_NUM_OF_RECORDS(block, H), BLOCK_DATA(block, H)};
}

/**
 * @brief Return an end iterator (position of the end) for the given block content.
 * @tparam H is the output size of the hash function.
 * @param block to iterate.
 * @return the end iterator.
 */
template <unsigned short H>
BlockRecordIterator blockRecordsEnd(const char* block){
    BlockRecordIterator it = blockRecordsBegin<H>(block);
    it.end();
    return it;
}


/**
 * @brief Return a record by the given position.
 * @tparam H is the output size of the hash function.
 * @param block to iterate.
 * @param index of the record.
 * @throw IndexOutOfBoundException in case where the index is bigger then the number of records inside the block.
 * @return pointer record.
 * @attention Do not free this pointer, it point inside the given block.
 */
template <unsigned short H>
const char* getBlockRecord(const char* block, size_t index){
    if(BLOCK_NUM_OF_RECORDS(block, H) <= index){
        throw IndexOutOfBoundException("BLOCK:getBlockRecord got %u when length is %u", index, BLOCK_NUM_OF_RECORDS(block, H));
    }
    BlockRecordIterator it = blockRecordsBegin<H>(block);
    for(int i=0; i < index; i++){
        it++;
    }
    return *it;
}


/**
 * @brief Add new record into the block.
 * @tparam H is the output size of the hash function.
 * @param block to insert the record into.
 * @param record is the record to insert.
 * @param len of the record.
 */
template <unsigned short H>
void addRecordContent(MemBuffer *block, const char *record, size_t len) {
    *(BLOCK_NUM_OF_RECORDS_POINTER(block->data(), H)) += 1;
    block->append(record, len);
}

/**
 * @brief Check if a block include a record from a specific participant.
 * @tparam H is the output size of the hash function.
 * @param block to check.
 * @param from is a buffer for the specific participant.
 * @param fromLen is the length of from.
 * @return true if the block include a record from the given particiapnt or false otherwise.
 */
template <unsigned short H>
bool hasRecordFrom(const char *block, const char* from, size_t fromLen){
    BlockRecordIterator it = blockRecordsBegin<H>(block);
    BlockRecordIterator end = blockRecordsEnd<H>(block);
    for(; it != end; it++){
        if(RECORD_FROM_LENGTH(*it) == fromLen && memcmp(RECORD_FROM(*it), from, fromLen) == 0){
            return true;
        }
    }
    return false;
}


/**
 * @brief Check if a block include a record from a specific participant.
 * @tparam H is the output size of the hash function.
 * @param block to check.
 * @param from is a buffer for the specific participant.
 * @param fromLen is the length of from.
 * @return true if the block include a record from the given participant or false otherwise.
 */
template <unsigned short H>
bool hasRecordFrom(const MemBuffer *block, const char* from, size_t fromLen){
    return hasRecordFrom<H>(block->data(), from, fromLen);
}


/**
 * @brief Check if a block include the given record.
 * @tparam H is the output size of the hash function.
 * @param block to check.
 * @param record is a buffer for the specific record.
 * @param len is the length of record.
 * @return true if the block include that record or false otherwise.
 */
template <unsigned short H>
bool hasRecord(MemBuffer *block, const char *record, size_t len) {
    BlockRecordIterator it = blockRecordsBegin<H>(block->data());
    BlockRecordIterator end = blockRecordsEnd<H>(block->data());
    for(; it != end; it++){
        if(RECORD_LENGTH(*it) == len && memcmp(*it, record, len) == 0)
        {
            return true;
        }
    }
    return false;
}




/*template<unsigned short H = BLOCKCHAIN_HASH_OUTPUT_SIZE>
class Block : public MemBuffer{
public:
    *
     * Copy constructor.
     * @param other pointer of Block instance.

    explicit Block(Block* other) : MemBuffer(other){}

    *
     * Serialize constructor.
     * @param buffer to load object from.
     * @param len of the buffer.

    Block(const char* data, size_t len) : MemBuffer(data, len){}

    *
     * Constructor.
     * @param blockId the number of block in the chain.
     * @param timestamp of the block
     * @param prevBlockHash the hash of the previous block in the chain, can be null.

    Block(size_t blockId, time_t timestamp, const char* prevBlockHash){
        reserveMore(sizeof(size_t) + sizeof(time_t) + H + sizeof(size_t));
        append(&blockId, sizeof(size_t));
        append(&timestamp, sizeof(time_t));
        if(prevBlockHash != nullptr) {
            append(prevBlockHash, H);
        }
        else{
            appendValue(0,H);
        }
        appendValue(0,sizeof(size_t));
    }

    size_t blockId() const{
        return *((size_t*)data());
    }

    time_t timestamp(){
        return *((time_t*)(data() + sizeof(size_t)));
    }

    const char* getPrevHash(){
        return data() + sizeof(size_t) + sizeof(time_t);
    }

    void setPrevHash(char hash[H]){
        memcpy(data() + sizeof(size_t) + sizeof(time_t), hash, H);
    }

    size_t numberOfRecords(){
        return *numberOfRecordsPointer();
    }

    void addContent(const char *content, size_t len)
    {
        (*numberOfRecordsPointer()) += 1;
        reserveMore(sizeof(size_t) + len);
        append(&len, sizeof(size_t));
        append(content, len);
    }

private:
    size_t* numberOfRecordsPointer(){
        return ((size_t*)(getPrevHash() + H));
    }
};*/


#endif //BLOCKCHAIN_BLOCK_H
