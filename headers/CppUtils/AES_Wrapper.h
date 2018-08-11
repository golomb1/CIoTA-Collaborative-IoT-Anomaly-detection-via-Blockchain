//
// Created by master on 18/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_AES_H
#define CIOTA_COMPLETE_PROJECT_AES_H

#include "../../matrixssl/crypto/cryptoApi.h"
#include "Exceptions.h"
#include "MemBuffer.h"
#include "CryptoLibrary.h"

#define AES_BLOCK_SIZE 16

template<uint8_t KEY_SIZE>
class AES_Wrapper {
public:
    static unsigned char *
    encryptCBC(const unsigned char key[KEY_SIZE], const unsigned char *pt, uint32_t ptLen, uint32_t *outLen) {
        int32 err;
        psAesCbc_t eCtx{};

        const unsigned char iv[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
                                      0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
        if ((err = psAesInitCBC(&eCtx, iv, key, KEY_SIZE, PS_AES_ENCRYPT)) != PS_SUCCESS) {
            throw MessageException("AES::encryptCBC::psAesInitCBC has failed with error code: %d", err);
        }

        *outLen = ptLen;
        if ((ptLen % AES_BLOCK_SIZE) > 0) {
            *outLen += (AES_BLOCK_SIZE - (ptLen % AES_BLOCK_SIZE));
        }
        unsigned char *ct = nullptr;
        ct = new unsigned char[*outLen];

        psAesEncryptCBC(&eCtx, pt, ct, ptLen);
        psAesClearCBC(&eCtx);
        return ct;
    }

    static unsigned char *
    decryptCBC(const unsigned char key[KEY_SIZE], const unsigned char *ct, uint32_t ctLen) {
        int32 err;
        psAesCbc_t dCtx{};

        auto *pt = new unsigned char[ctLen];
        const unsigned char iv[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
                                      0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
        if ((err = psAesInitCBC(&dCtx, iv, key, KEY_SIZE, PS_AES_DECRYPT)) != PS_SUCCESS) {
            throw MessageException("AES::encryptCBC::psAesInitCBC has failed with error code: %d", err);
        }
        psAesDecryptCBC(&dCtx, ct, pt, ctLen);
        psAesClearCBC(&dCtx);
        return pt;
    }
};

class AES_SEALER : public SealerFunction {
private:
    MemBuffer const * _id;
    char seed[32];
public:
    explicit AES_SEALER(MemBuffer* identification) : _id(identification), seed{}{
        memset(seed, 0, 32);
    }

    const MemBuffer *getIdentifier() const override {
        return _id;
    }

    void SEED(unsigned char s[32]) {
        memset(seed, 0, 32);
        memcpy(seed, s, 32);
    }

    bool seal(const char *from, size_t fromLen, const char *content, size_t contentLen, MemBuffer *out) {
        unsigned char key[32];
        memset(key, 0, 32);
        size_t min = fromLen;
        if (min > 32) {
            min = 32;
        }
        memcpy(key, from, min);
        for (int i = 0; i < 32; i++) {
            key[i] = key[i] ^ seed[i];
        }
        uint32_t outLen;

        auto signedContentLen = static_cast<uint32_t>(8 + contentLen + sizeof(size_t));
        auto *signedContent = new unsigned char[signedContentLen];
        memset(signedContent, 0, signedContentLen);
        memcpy(signedContent + 8 + sizeof(size_t), content, contentLen);
        *((size_t *) (signedContent + 8)) = contentLen;
        unsigned char *s = AES_Wrapper<32>::encryptCBC(key, signedContent, signedContentLen, &outLen);
        bool res = s != nullptr;
        if (res) {
            out->append(s, outLen);
        }
        delete[] s;
        delete[] signedContent;
        return res;
    }


    bool unseal(const char *from, size_t fromLen, const char *content, size_t contentLen, MemBuffer *out) override {
        unsigned char key[32];
        memset(key, 0, 32);
        size_t min = fromLen;
        if (min > 32) {
            min = 32;
        }
        memcpy(key, from, min);
        for (int i = 0; i < 32; i++) {
            key[i] = key[i] ^ seed[i];
        }

        uint32_t outLen;
        unsigned char zeros[8];
        memset(zeros, 0, 8);
        unsigned char *s = AES_Wrapper<32>::decryptCBC(key,
                                                       reinterpret_cast<const unsigned char *>(content),
                                                       static_cast<uint32_t>(contentLen));
        if (s == nullptr || memcmp(s, zeros, 8) != 0 || *((size_t *) (s + 8)) > contentLen) {
            delete[] s;
            return false;
        }
        size_t len = *((size_t *) (s + 8));
        out->append(s + 8 + sizeof(size_t), len);
        delete[] s;
        return true;
    }

};

#endif //CIOTA_COMPLETE_PROJECT_AES_H
