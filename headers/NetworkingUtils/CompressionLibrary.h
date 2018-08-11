//
// Created by golombt on 20/12/2017.
//

#ifndef NETWORKING_UTILS_COMPRESSION_LIBRARY_H
#define NETWORKING_UTILS_COMPRESSION_LIBRARY_H

#include "../CppUtils/MemBuffer.h"
#include "../CppUtils/minilzo.h"

class CompressionLibrary{
public:
    virtual char *
    getCompressedData(const char *message, size_t len, size_t *outLen, bool *shouldFree) noexcept(false) = 0;

    virtual char *
    getDecompressedData(const char *message, size_t len, size_t *outLen, bool *shouldFree) noexcept(false) = 0;
};


class MiniLZOLibrary : public CompressionLibrary{
public:
    char *getCompressedData(const char *message, size_t len, size_t *outLen, bool *shouldFree) noexcept(false) override;

    char *
    getDecompressedData(const char *message, size_t len, size_t *outLen, bool *shouldFree) noexcept(false) override;
};

class MockCompressionLibrary : public CompressionLibrary{
public:
    char *getCompressedData(const char *message, size_t len, size_t *outLen, bool *shouldFree) noexcept(false) override;

    char *
    getDecompressedData(const char *message, size_t len, size_t *outLen, bool *shouldFree) noexcept(false) override;
};

#endif //NETWORKING_UTILS_COMPRESSION_LIBRARY_H
