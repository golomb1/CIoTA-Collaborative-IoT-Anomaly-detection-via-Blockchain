//
// Created by golombt on 17/12/2017.
//

#ifndef CPP_UTILS_CURSOR_H
#define CPP_UTILS_CURSOR_H


#include <vector>
#include <cstring>


/**
 *  @file    Cursor.h
 *  @author  Tomer Golomb (golombt)
 *  @version 1.0
 *
 *  @brief defines an interface for Cursor class.
 *  Cursor is used to ease the handling of parsing bye buffers.
 *
 */
class Cursor{
public:

    /**
     * Constructor for cursor from a given buffer.
     * @param buffer to be read.
     * @param len of the buffer.
     * @attention buffer is NOT copied and should not be freed by the caller.
     */
    Cursor(const void* buffer, size_t len);

    virtual ~Cursor() = default;

    /**
     * Get pointer to the head of the cursor.
     * @return
     */
    const char* head();

    /**
     * check whether there is at least n more bytes to read.
     * @param n the size ot check.
     * @return true if there is, and false otherwise.
     */
    bool has(size_t n);

    /**
     * @return whether there is more bytes to read.
     */
    bool hasMore();

    /**
     * @return how many byts available to read.
     */
    size_t left();

    /**
     * return the current offset and advance the cursor by n.
     * @param n - the advance of the cursor.
     * @return the offset before advancing.
     */
    size_t takeOffset(size_t n);

    /**
     * return the current head pointer and advance the cursor by n.
     * @param n
     * @return the head pointer before advancing.
     */
    const char* take(size_t n);

    /**
     * Read n n bytes into a given buffer and advance the cursor.
     * @param buffer to write into.
     * @param n - how many to write.
     */
    void readInto(void* buffer, size_t n);



private:
    const char*  _buffer;
    size_t       _remaining;
    size_t       _totalSize;
};



#endif //CPP_UTILS_CURSOR_H
