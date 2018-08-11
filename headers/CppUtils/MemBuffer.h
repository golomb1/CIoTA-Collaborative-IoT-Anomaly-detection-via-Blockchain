//
// Created by golombt on 17/12/2017.
//

#ifndef CPP_UTILS_MEM_BUFFER_H
#define CPP_UTILS_MEM_BUFFER_H

#include <vector>
#include <cstdio>
#include <string>

/**
 *  @file    MemBuffer.h
 *  @author  Tomer Golomb (golombt)
 *  @version 1.0
 *
 *  @brief defines an interface for MemBuffer class.
 *  MemBuffer is used to ease the handling of memory buffers.
 *  Support dynamic allocation and resizing using std vector.
 *
 */


class MemBuffer{
private:
    std::vector<char> _inner;
public:

    /**
     * Constructor
     */
    MemBuffer() = default;


    /**
     * Constructor copy the given buffer to this instance.
     * @param buffer to be copied
     * @param len of the buffer.
     * @attention buffer is copied and should be freed by the caller.
     */
    MemBuffer(const void* buffer, size_t len){
        append(buffer, len);
    }

    /**
     * Copy constructor for pointer.
     * @param other instance to copy.
     */
    explicit MemBuffer(MemBuffer* other) : MemBuffer(other->_inner.data(), other->_inner.size()){}

    explicit MemBuffer(std::string str);

    explicit MemBuffer(std::string* str);


    virtual ~MemBuffer(){
        _inner.clear();
    }

    /**
     * append data from ptr into this buffer.
     * @param ptr to the data.
     * @param len to read from ptr.
     * @attention  ptr is copied and should be freed by the caller.
     */
    void append(const void* ptr, size_t len);

    /**
     * reserve n more bytes in this buffer.
     * @param n - how many byte to reserve.
     */
    void reserveMore(size_t n);

    /**
     * append a char value n times
     * @param value to append
     * @param n - how many times to write the value.
     */
    void appendValue(char value, size_t n);


    /**
     * Pad this buffer to the desired length with a given value.
     * In a case where the length of the buffer is already equal
     * or greater than the desired length nothing occurs.
     * @param value to pad with.
     * @param desiredLength of the buffer.
     */
    void padValueTo(char value, size_t desiredLength);

    const char *data() const;

    char *data();

    size_t size() const;

    void clear();

    bool empty() const ;

    void swap(std::vector<char>& v);

    void swap(MemBuffer& m);

};

#endif //CPP_UTILS_MEM_BUFFER_H
