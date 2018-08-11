//
// Created by golombt on 21/12/2017.
//

#include <cstdio>
#include "../../headers/CppUtils/Exceptions.h"
#include "../../headers/NetworkingUtils/CompressionLibrary.h"


static lzo_align_t __LZO_MMODEL wrkmem  [ (( LZO1X_1_MEM_COMPRESS) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ];


/**
 * This method create compression buffer  <original size + compressed data>.
 */
char *MiniLZOLibrary::getCompressedData(const char *message, size_t len, size_t *outLen, bool *shouldFree) {
    if (lzo_init() != LZO_E_OK)
    {
        throw MessageException("MiniLZOLibrary::getCompressedData - internal error - lzo_init() failed !!!");
    }
    // compression is <original size + compressed data>
    lzo_uint outLen2 = *outLen;
    auto *outputBuffer = new char[len + sizeof(size_t)];
    int result = lzo1x_1_compress(reinterpret_cast<const unsigned char *>(message), len,
                                  reinterpret_cast<unsigned char *>(outputBuffer + sizeof(size_t)), &outLen2, wrkmem);
    *outLen = outLen2;
    if (result != LZO_E_OK) {
        throw MessageException("MiniLZOLibrary::getCompressedData - internal error - compression failed: %d\n", result);
    }
    /* check for an incompressible block */
    if (*outLen >= len)
    {
        delete[] (outputBuffer);
        *shouldFree = false;
        *outLen = len;
        return nullptr;
    }
    else{
        *((size_t *) outputBuffer) = len;
        *shouldFree = true;
        return outputBuffer;
    }
}

char *
MiniLZOLibrary::getDecompressedData(const char *message, size_t len, size_t *outLen, bool *shouldFree) noexcept(false) {
    if (lzo_init() != LZO_E_OK)
    {
        throw MessageException("MiniLZOLibrary::getDecompressedData - internal error - lzo_init() failed !!!");
    }
    if (len < sizeof(size_t)) {
        *shouldFree = false;
        return nullptr;
    } else {
        *outLen = *((size_t *) message);
        auto *out = new char[*outLen];
        lzo_uint outLen2 = *outLen;
        int r = lzo1x_decompress(reinterpret_cast<const unsigned char *>(message + sizeof(size_t)),
                                 len - sizeof(size_t),
                                 reinterpret_cast<unsigned char *>(out), &outLen2, wrkmem);
        *outLen = outLen2;
        if (r != LZO_E_OK) {
            throw MessageException("MiniLZOLibrary::getDecompressedData - internal error - compression failed: %d\n",
                                   r);
        }
        /* check for an incompressible block */
        if (*outLen < len) {
            *shouldFree = false;
            *outLen = len;
            return nullptr;
        } else {
            *shouldFree = true;
            return out;
        }
    }
}

char *
MockCompressionLibrary::getCompressedData(const char *, size_t len, size_t *outLen, bool *shouldFree) {
    *shouldFree = false;
    *outLen = len;
    return nullptr;
}

char *
MockCompressionLibrary::getDecompressedData(const char *, size_t len, size_t *outLen, bool *shouldFree) {
    *shouldFree = false;
    *outLen = len;
    return nullptr;
}
