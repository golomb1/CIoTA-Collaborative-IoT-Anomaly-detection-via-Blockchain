//
// Created by golombt on 27/12/2017.
//

#ifndef CIOTA_COMPLETE_PROJECT_CRYPTO_LIBRARY_H
#define CIOTA_COMPLETE_PROJECT_CRYPTO_LIBRARY_H


#include <sstream>
#include "MemBuffer.h"
#include "SHA256.h"


template <unsigned short H>
class HashFunction {
public:
    /**
     * @brief calculate an hash value of the given message.
     * @param out is the output buffer.
     * @param message is the message to hash.
     * @param len is the length of the message.
     */
    virtual void hash(char out[H], const char *message, size_t len) = 0;
};


class SealerFunction {
public:
    /**
     * Unseal a buffer.
     * @param from is identifier of the sealer, should be used to unseal to buffer.
     * @param fromLen is the length of from.
     * @param content is the content to unseal.
     * @param contentLen is the length of the content.
     * @param out is the output buffer.
     * @return true if success or false otherwise.
     */
    virtual bool unseal(const char* from, size_t fromLen, const char* content, size_t contentLen, MemBuffer* out) = 0;

    /**
     * Seal a buffer.
     * @param from is identifier of the sealer, should be used to unseal to buffer.
     * @param fromLen is the length of from.
     * @param content is the content to seal.
     * @param contentLen is the length of the content.
     * @param out is the output buffer.
     * @return true if success or false otherwise.
     */
    virtual bool seal(const char *from, size_t fromLen, const char *content, size_t contentLen, MemBuffer *out) = 0;

    /**
     * @return the sealer identification
     */
    virtual const MemBuffer *getIdentifier() const = 0;
};


/**
 * @brief Defines an interface for cryptographic library.
 * @tparam H is the output size of the hash function.
 */
template<unsigned short H>
class CryptoLibrary : public HashFunction<H>, public SealerFunction {
private:
    HashFunction<H> *_hash;
    SealerFunction *_sealer;
public:
    CryptoLibrary(HashFunction<H> *_hash, SealerFunction *_sealer) : _hash(_hash), _sealer(_sealer) {}

    void hash(char *out, const char *message, size_t len) override {
        _hash->hash(out, message, len);
    }

    bool unseal(const char *from, size_t fromLen, const char *content, size_t contentLen, MemBuffer *out) override {
        return _sealer->unseal(from, fromLen, content, contentLen, out);
    }

    bool seal(const char *from, size_t fromLen, const char *content, size_t contentLen, MemBuffer *out) override {
        return _sealer->seal(from, fromLen, content, contentLen, out);
    }


    const MemBuffer *getIdentifier() const override {
        return _sealer->getIdentifier();
    }
};


class SHA256_FUNC : public HashFunction<SHA256_OUTPUT_SIZE> {
public:
    void hash(char out[SHA256_OUTPUT_SIZE], const char *message, size_t len) override {
        std::stringstream ss;
        for(size_t i(0);i<len;++i) {
            ss << std::hex << (int) (message[i]);
        }
        std::string sha = sha256(ss.str());
        memset(out, 0, SHA256_OUTPUT_SIZE);
        size_t min = sha.size();
        if(min > SHA256_OUTPUT_SIZE){
            min = SHA256_OUTPUT_SIZE;
        }
        memcpy(out, sha.data(), min);
    }
};




#endif //CIOTA_COMPLETE_PROJECT_CRYPTO_LIBRARY_H
