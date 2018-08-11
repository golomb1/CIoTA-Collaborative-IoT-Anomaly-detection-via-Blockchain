//
// Created by golombt on 17/12/2017.
//

#ifndef BLOCKCHAIN_RECORD_H
#define BLOCKCHAIN_RECORD_H

#include "../CppUtils/MemBuffer.h"


/**
 *  @file    BlockRecord.h
 *  @author  Tomer Golomb (golombt)
 *  @version 1.0
 *
 *  @brief defines an interface for BlockRecord class.
 *  BlockRecord is a buffer that represents a blockchain record.
 *  Each record is composed from two fields: <From><Content>
 *  Where <From> is the record owner (the one that created it).
 *  Where <Content> is the record content (the one that created it).
 *  For example, in bitcoin <From> is the public key of the one performing the transaction
 *  and <Content> is the transaction (The receiver and how much money to transfer).
 *
 */


/**
 * This structure was created to define the header of a record.
 */
typedef struct BlockRecordHeader{
    size_t fromLength;
    size_t contentLength;
} BlockRecordHeader;



/* Defines getters for record content */
#define RECORD_FROM_LENGTH(record)    (((BlockRecordHeader*)(record))->fromLength)
#define RECORD_CONTENT_LENGTH(record) (((BlockRecordHeader*)(record))->contentLength)
#define RECORD_FROM(record)           ((record) + sizeof(BlockRecordHeader))
#define RECORD_CONTENT(record)        (RECORD_FROM(record) + RECORD_FROM_LENGTH(record))
#define RECORD_LENGTH(record)         (RECORD_FROM_LENGTH(record)+RECORD_CONTENT_LENGTH(record)+sizeof(BlockRecordHeader))



class BlockRecord : public MemBuffer {
public:
    /**
     * @brief Copy constructor.
     * @param other pointer of BlockRecord instance.
     */
    explicit BlockRecord(BlockRecord* other);

    /**
     * @brief Serialize constructor.
     * @param buffer to load object from.
     * @param len of the buffer.
     */
    BlockRecord(const char* buffer, size_t len);

    /**
     * @brief Constructor, create a new record object.
     * @param from buffer for the from field.
     * @param fromLength the from buffer length.
     * @param seed of the record.
     * @param seedLength the seed length.
     * @param content of the record.
     * @param contentLength the content length.
     */
    BlockRecord(const char* from, size_t fromLength,
                const char* content, size_t contentLength);

    /* Getters */
    size_t fromLength();
    size_t contentLength();
    const char* from();
    const char* content();
};


#endif //BLOCKCHAIN_RECORD_H
