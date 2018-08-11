//
// Created by golombt on 17/12/2017.
//

#include <iostream>
#include "../../headers/CppUtils/MemBuffer.h"





void MemBuffer::append(const void *ptr, size_t len) {
    if(_inner.capacity() < _inner.size() + len){
        reserveMore(size() + len - _inner.capacity());
    }
    for(size_t i = 0 ; i < len; i++){
        _inner.push_back(((char*)ptr)[i]);
    }
}


void MemBuffer::reserveMore(size_t n) {
    _inner.reserve(_inner.capacity() + n);
}

void MemBuffer::appendValue(char value, size_t n) {
    if(_inner.capacity() < _inner.size() + n){
        reserveMore(_inner.size() + n - _inner.capacity());
    }
    for(size_t i = 0 ; i < n; i++){
        _inner.push_back(value);
    }
}

void MemBuffer::padValueTo(char value, size_t desiredLength) {
    if(desiredLength > _inner.size()) {
        appendValue(value, desiredLength - _inner.size());
    }
}

const char *MemBuffer::data() const {
    return _inner.data();
}

char *MemBuffer::data() {
    return _inner.data();
}

size_t MemBuffer::size() const {
    return _inner.size();
}

void MemBuffer::clear() {
    return _inner.clear();
}

bool MemBuffer::empty() const {
    return _inner.empty();
}

void MemBuffer::swap(std::vector<char>& v) {
    _inner.swap(v);
}

void MemBuffer::swap(MemBuffer& m) {
    _inner.swap(m._inner);
}

MemBuffer::MemBuffer(std::string str) : MemBuffer(str.c_str(), str.size()) {}

MemBuffer::MemBuffer(std::string *str) : MemBuffer(str->c_str(), str->size()) { }

