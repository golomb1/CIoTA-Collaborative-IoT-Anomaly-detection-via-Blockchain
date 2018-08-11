//
// Created by golombt on 24/12/2017.
//

#ifndef CPP_UTILS_CIRCULAR_BUFFER_H
#define CPP_UTILS_CIRCULAR_BUFFER_H

#include <cstdio>
#include "Exceptions.h"
#include <iostream>

template <typename T>
class CircularBuffer {
private:
    T* _array;
    size_t _index;
    size_t _capacity;
    bool _full;

public:
    explicit CircularBuffer(size_t capacity) : _full(false), _array(new T[capacity]), _index(0), _capacity(capacity) {  }

    CircularBuffer(const CircularBuffer& other)
            : _full(other._full), _array(new T[other._capacity]), _index(other._index), _capacity(other._capacity)
    {
        for(int i=0; i < other._capacity; i++){
            _array[i] = other._array[i];
        }
    }


    virtual ~CircularBuffer(){
        if(_array != nullptr) {
            delete[] _array;
            _array = nullptr;
        }
    }
    
    /**
    * Return the item at the given positions from the head of the buffer.
    * throw IndexOutOfBoundException in cases where the position is invalid or the buffer is empty.
    * @param i - the requested position.
    * @return the item at the given positions from the head of the buffer.
    */    
    T peek(size_t i){
        if(i >= size()){
            throw IndexOutOfBoundException("CircularBuffer:peek - Index out of bound");
        }
        if(_full){
            return _array[(_index - 1 - i + 2*_capacity) % _capacity];
        }
        else{
            return _array[size() - 1 - i];
        }
    }
    
    /**
    * Push a new item into this buffer.
    */
    void push_back(T& item){
        _array[_index] = item;
        _full |= (_index + 1 == _capacity);
        _index = (_index + 1)%_capacity;
    }

    /**
    * Return the last item that is saved by this buffer.
    * throw IndexOutOfBound exception in cases where the buffer is empty.
    * @return the last item that is saved by this buffer.
    */
    T last(){
        return fromLast(0);
    }
    
    
    size_t size(){
        return _full ? _capacity : _index;
    }

    size_t capacity() const{
        return _capacity;
    }
    
    T fromLast(size_t i){
        return peek(size() - 1 - i);
    }
};


#endif //CPP_UTILS_CIRCULAR_BUFFER_H
