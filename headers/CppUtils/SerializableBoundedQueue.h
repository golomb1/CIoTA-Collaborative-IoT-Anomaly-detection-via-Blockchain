//
// Created by golombt on 17/12/2017.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma ide diagnostic ignored "OCUnusedStructInspection"
#ifndef CPP_UTILS_BOUND_BUFFER_H
#define CPP_UTILS_BOUND_BUFFER_H

#include <cstdio>
#include <algorithm>
#include <iostream>
#include "MemBuffer.h"
#include "Exceptions.h"

#define HEAD_INDEX(b) (*((size_t*)(b)))
#define NEXT_INDEX(b, c) (HEAD_INDEX(b) + ((2*sizeof(size_t)) + (*((size_t*)((b)+sizeof(size_t)))))) % (c)
#define BUFFER(b) ((b) + sizeof(size_t))


typedef struct SerializableBoundedQueueHeaders{
    size_t nextPointer;
    size_t size;
} SerializableBoundedQueueHeaders;


/**
 *  TODO make this more efficient.
 *  @file    SerializableBoundedBuffer.h
 *  @author  Tomer Golomb (golombt)
 *  @version 1.0
 *
 *  @brief This file defines an interface and implementation for SerializableBoundedBuffer class.
 *  SerializableBoundedBuffer is a cyclic buffer of chars that removes old entries to insert new ones.
 *  Therefore, the buffer does not support explicit remove of items but overriding them.
 *  Support std functions such as push_back, peek, head, size.
 *
 *  This object is construct as follows:
 *          [size_t] the offset of the queue head.
 *          [buffer of items] - the items.
 *
 *  Each item is constructed as:
 *          [size_t] next item offset
 *          [size_t] size of record
 *          [CHAR[size of record]] data
 */
class SerializableBoundedQueue {
private:
    char*               _buffer;
    const char*         _constBuffer;
    size_t              _capacity;
    std::vector<size_t> _offsets{};
    size_t              _nextIndex;

public:

    /**
     * Constructor
     * @param capacity is the max memory to allocate for the buffer.
     */
    explicit SerializableBoundedQueue(size_t capacity) :
            _buffer(new char[capacity + sizeof(size_t)]),
            _constBuffer(_buffer),
            _capacity(capacity),
            _offsets(0, 0),
            _nextIndex(0)
    {
        memset(_buffer, 0, capacity + sizeof(size_t));
        memset(_buffer, 1, sizeof(size_t));
    }

    /**
     * Copy constructor
     * @param other instance to copy.
     */
    explicit SerializableBoundedQueue(const SerializableBoundedQueue* other)
            : _buffer(new char[other->_capacity + sizeof(size_t)]),
              _constBuffer(_buffer),
              _capacity(other->_capacity),
              _offsets(other->_offsets),
              _nextIndex(other->_nextIndex)
    {
        memcpy(_buffer, other->_buffer, _capacity + sizeof(size_t));
    }

    /**
     * Serialize constructor
     * @param buffer to use.
     * @param len of buffer.
     */
    SerializableBoundedQueue(const char* buffer, size_t len)
            : _buffer(nullptr),
              _constBuffer(buffer),
              _capacity(len - sizeof(size_t)),
              _offsets(),
              _nextIndex(NEXT_INDEX(buffer, _capacity))
    {
        size_t offset = HEAD_INDEX(_constBuffer);
        if(offset != ~0) {
            while (offset != 0) {
                auto *headers = (SerializableBoundedQueueHeaders *) (BUFFER(_constBuffer) + offset);
                _offsets.insert(_offsets.begin(), offset);
                offset = headers->nextPointer;
            }
            _offsets.insert(_offsets.begin(), offset);
        }
    }

    virtual ~SerializableBoundedQueue(){
        if(_buffer != nullptr) {
            delete[] _buffer;
            _buffer = nullptr;
        }
        _offsets.erase(_offsets.begin(), _offsets.end());
        _offsets.clear();
        std::vector<size_t>().swap(_offsets);
    }



    /**
     * Push new item to the queue.
     * @param item to be pushed.
     * @param len of the item.
     */
    void push_back(const char* item, size_t len) {
        if(len + sizeof(SerializableBoundedQueueHeaders) > _capacity) {
            throw IndexOutOfBoundException(
                    "SerializableBoundedStack::push_back - receives value of size bigger than the capacity", len,
                    _capacity);
        } else {
            // all the content fit inside without cycle.
            // remove all the data from endIndex to len
            size_t eraseFrom = _nextIndex;
            size_t eraseTo = eraseFrom + len + sizeof(SerializableBoundedQueueHeaders);
            size_t eraseFrom2 = eraseFrom;
            size_t eraseTo2 = eraseTo;
            if (eraseTo >= _capacity) {
                eraseFrom2 = 0;
                eraseTo2 = len + sizeof(SerializableBoundedQueueHeaders);
            }
            size_t eraseUntil = _offsets.size() + 1;
            for (size_t i = 0; i < _offsets.size() &&
                               ((_offsets.at(i) >= eraseFrom  && _offsets.at(i) < eraseTo) ||
                                (_offsets.at(i) >= eraseFrom2 && _offsets.at(i) < eraseTo2)); i++) {
                eraseUntil = i;
            }
            if (eraseUntil + 1 <=  _offsets.size()) {
                _offsets.erase(_offsets.begin(), _offsets.begin() + eraseUntil+1);
            }
            else if (eraseUntil < _offsets.size()) {
                _offsets.erase(_offsets.begin(), _offsets.begin() + eraseUntil);
            }
            if (!_offsets.empty()) {
                *((size_t *) (BUFFER(_buffer) + _offsets.at(0))) = 0;
            }
            size_t prevOffset = 0;
            if (!_offsets.empty()) {
                prevOffset = _offsets.at(_offsets.size() - 1);
            }
            (*((SerializableBoundedQueueHeaders*)(BUFFER(_buffer) + eraseFrom2))) = {prevOffset, len};
            memcpy(BUFFER(_buffer) + eraseFrom2 + sizeof(SerializableBoundedQueueHeaders), item, len);
            _offsets.push_back(eraseFrom2);
            _nextIndex = (eraseFrom2 + len + sizeof(SerializableBoundedQueueHeaders)) % _capacity;
            HEAD_INDEX(_buffer) = eraseFrom2;
        }
    }


    /**
     * Pop the newest item from the queue.
     */
    void removeHead() {
        if(size() == 0) {
            throw IndexOutOfBoundException(
                    "SerializableBoundedStack::removeHead - on empty queue");
        } else {
            size_t offset = _offsets.at(_offsets.size() - 1);
            _offsets.pop_back();
            _nextIndex = offset;
            if(_offsets.size() > 1) {
                HEAD_INDEX(_buffer) = _offsets.at(_offsets.size() - 1);
            }
            else{
                memset(&(HEAD_INDEX(_buffer)), 1, sizeof(size_t));
            }
            //rearrange();
        }
    }

    void removeTail(){
        if(size() == 0) {
            throw IndexOutOfBoundException(
                    "SerializableBoundedStack::removeTail - on empty queue");
        } if(size() == 1){
            removeHead();
        } else {
            _offsets.erase(_offsets.begin());
            size_t offset =_offsets.at(0);
            ((SerializableBoundedQueueHeaders*)(BUFFER(_buffer) + _offsets.at(0)))->nextPointer = 0;
            //rearrange();
        }
    }

    /**
     * @return the index-th oldest item in this queue (the index-th item from the tail).
     * @throw IndexOutOfBoundException in case the buffer doesn't have such element.
     */
    const char* at(size_t index) const {
        if(index >= _offsets.size()){
            throw IndexOutOfBoundException("SerializableBoundedStack::at", index, _offsets.size());
        }
        return (BUFFER(_constBuffer) + sizeof(SerializableBoundedQueueHeaders) + _offsets.at(index));
    }

    /**
     * @return the length of the index-th oldest item in this queue (the index-th item from the tail).
     * @throw IndexOutOfBoundException in case the buffer doesn't have such element.
     */
    size_t lengthAt(size_t index) const{
        if(index >= _offsets.size()){
            throw IndexOutOfBoundException("SerializableBoundedStack::lengthAt", index, _offsets.size());
        }
        return ((SerializableBoundedQueueHeaders*)(BUFFER(_constBuffer) + _offsets.at(index)))->size;
    }

     /**
      * @return the next pointer of the index-th oldest item in this queue (the index-th item from the tail).
      *         if the index is zero (the last element = tail) zero should be returned.
      * @throw IndexOutOfBoundException in case the buffer doesn't have such element.
      */
    size_t getNextPointerAt(size_t index) const{
        if(index >= _offsets.size()){
            throw IndexOutOfBoundException("SerializableBoundedStack::getNextPointerAt", index, _offsets.size());
        }
        return ((SerializableBoundedQueueHeaders*)(BUFFER(_constBuffer) + _offsets.at(index)))->nextPointer;
    }

    /**
     * @return the next pointer of the index-th newest item in this queue (the index-th item from the head).
     *         if the index is zero (the last element = tail) zero should be returned.
     * @throw IndexOutOfBoundException in case the buffer doesn't have such element.
     */
    size_t getNextPointerPeek(size_t index) const{
        return getNextPointerAt(_offsets.size() - 1 - index);
    }

    /**
     * @return the newest item in this queue.
     * @throw IndexOutOfBoundException in case the buffer is empty.
     */
    const char* head() const{
        return peek(0);
    }

    /**
     * @return the length of the head record.
     * @throw IndexOutOfBoundException in case the buffer is empty.
     */
    size_t headLength() const{
        return lengthOfPeek(0);
    }

    /**
     * @return the index-th item in this queue.
     * @throw IndexOutOfBoundException in case the buffer doesn't have such element.
     */
    const char* peek(size_t index) const {
        return at(_offsets.size()-1 - index);
    }

    /**
     * @return the length of the index-th item in this queue.
     * @throw IndexOutOfBoundException in case the buffer doesn't have such element.
     */
    size_t lengthOfPeek(size_t index) const{
        return lengthAt(_offsets.size()-1 - index);
    }

    /**
     * @return the oldest item in this queue.
     * @throw IndexOutOfBoundException in case the buffer is empty.
     */
    const char* tail() const{
        return at(0);
    }

    /**
     * @return the length of the trail record.
     * @throw IndexOutOfBoundException in case the buffer is empty.
     */
    size_t tailLength() const{
        return lengthAt(0);
    }

    /**
     * @return vector of all the offsets of the items in this buffer.
     */
    const std::vector<size_t> getOffsets() const{
        return _offsets;
    }

    /**
     * @return the number of element in this queue.
     */
    size_t size() const {
        return _offsets.size();
    }

    // TODO revisit


    const char* serialize(){
        return _constBuffer;
    }

    size_t serializationSize(){
        return _capacity;
    }


    void rearrange(){
        /*if(size() > 0) {
            size_t itemOffset = _offsets.at(0);
            size_t nextOffset = getNextPointerAt(0);
            size_t itemSize = lengthAt(0);
            for (size_t i = 0; i < size(); i++) {
                size_t newOffset = itemOffset + itemSize + sizeof(SerializableBoundedQueueHeaders);
                itemSize = lengthAt(i);
                itemOffset = _offsets.at(i);
                if(newOffset + itemSize > _capacity){
                    newOffset = 0;
                }
                memcpy(BUFFER(_buffer) + newOffset, BUFFER(_buffer) + itemOffset, itemSize + sizeof(SerializableBoundedQueueHeaders));
                _offsets.at(i) = newOffset;
                ((SerializableBoundedQueueHeaders*)(BUFFER(_buffer) + newOffset))->nextPointer = nextOffset;
                nextOffset = getNextPointerAt(i);
                HEAD_INDEX(_buffer) = _nextIndex;
                _nextIndex = newOffset + itemSize;
            }
        }*/
    }

    size_t capacity() {
        return _capacity;
    }
};


#endif //CPP_UTILS_BOUND_BUFFER_H

#pragma clang diagnostic pop