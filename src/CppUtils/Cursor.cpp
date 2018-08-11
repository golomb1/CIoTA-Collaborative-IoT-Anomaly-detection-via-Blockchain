//
// Created by golombt on 17/12/2017.
//

#include "../../headers/CppUtils/Cursor.h"
#include "../../headers/CppUtils/Exceptions.h"

Cursor::Cursor(const void *buffer, size_t len) {
    _buffer = (char*)buffer;
    _remaining = len;
    _totalSize = len;
}


const char *Cursor::head() {
    return _buffer;
}

size_t Cursor::takeOffset(size_t n) {
    if(_remaining < n){
        throw IndexOutOfBoundException("Cursor", n, _remaining);
    }
    size_t retValue = _totalSize - _remaining;
    _remaining -= n;
    _buffer += n;
    return retValue;
}

bool Cursor::has(size_t n) {
    return _remaining >= n;
}

bool Cursor::hasMore() {
    return _remaining > 0;
}

size_t Cursor::left() {
    return _remaining;
}

const char *Cursor::take(size_t n) {
    if(_remaining < n){
        throw IndexOutOfBoundException("Cursor", n, _remaining);
    }
    const char *retValue = _buffer;
    _remaining -= n;
    _buffer += n;
    return retValue;
}

void Cursor::readInto(void *buffer, size_t n) {
    memcpy(buffer, take(n), n);
}
