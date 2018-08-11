//
// Created by master on 10/01/18.
//

#include <iostream>
#include "TestSuite1.h"
#include "../TEST.h"
#include "../../headers/CppUtils/CircularBuffer.h"
#include "../../headers/CppUtils/AES_Wrapper.h"
#include "../../headers/CppUtils/SerializableBoundedQueue.h"
#include "../Mocks/BlockchainTestUtils.h"
#include "Blockchain/PartialBlockTest.h"
#include "Blockchain/receiveChainTest.h"
#include "../Mocks/NetworkingUtilTests.h"
#include "CIoTA/CIoTA_TestApplication.h"
#include "CIoTA/CIoTA_TestAgent.h"
#include "JumpTracer/JumpTracerTest.h"


bool TestCircularBuffer(){
    std::cout << "TestCircularBuffer\n";
    CircularBuffer<int> buffer(10);
    for(int i=0; i < 10; i++){
        buffer.push_back(i);
        ASSERT(buffer.last(), 0);
    }
    for(int i=0; i < 10; i++) {
        for(int j=0; j < 10; j++) {
            buffer.push_back(j);
            ASSERT(buffer.last(), (1+j)%10);
        }
    }

    return true;
}

bool TestBuffer() {
    SerializableBoundedQueue buffer((2*sizeof(size_t)+1) * 5);
    size_t prevPrev = 0;
    size_t prev = 0;
    for(char i=0; i < 4; i++){
        char c = 'a' + i;
        buffer.push_back(&c, 1);
        auto ii = static_cast<size_t>(i + 1);
        ASSERT(buffer.size(), ii);
        ASSERT(buffer.head()[0], c);
        ASSERT(buffer.getNextPointerPeek(0), prev);
        if(i < 3) {
            prevPrev = prev;
        }
        prev = buffer.getOffsets().at(buffer.getOffsets().size() - 1);
    }
    buffer.removeHead();
    ASSERT(buffer.size(), 3);
    ASSERT(buffer.head()[0], 'a' + 2);
    ASSERT(buffer.getNextPointerPeek(0), prevPrev);
    prev = buffer.getOffsets().at(buffer.getOffsets().size() - 1);

    char c = 'a' + 3;
    buffer.push_back(&c, 1);
    auto ii = static_cast<size_t>(3 + 1);
    ASSERT(buffer.size(), ii);
    ASSERT(buffer.head()[0], c);
    ASSERT(buffer.getNextPointerPeek(0), prev);
    prev = buffer.getOffsets().at(buffer.getOffsets().size() - 1);

    const char* str = "hi";
    buffer.push_back(str, 2);
    ASSERT(buffer.size(), 3)
    ASSERT(memcmp(buffer.head(), str, 2), 0);
    ASSERT(buffer.getNextPointerPeek(0), prev);
    prev = buffer.getOffsets().at(buffer.getOffsets().size() - 1);
    char str2[20];
    memset(str2, '1', 20);
    buffer.push_back(str2, 20);
    ASSERT(buffer.size(), 2)
    ASSERT(memcmp(buffer.head(), str2, 20), 0);
    ASSERT(buffer.getNextPointerPeek(0), prev);

    char str3[3+2+16];
    memset(str3, '1', 3+2+16);
    buffer.push_back(str3, 3+2+16);

    ASSERT(buffer.size(), 1)
    ASSERT(memcmp(buffer.head(), str3, 3+2+16), 0);
    ASSERT(buffer.getNextPointerPeek(0), 0);
    return true;

    /*

    for(size_t of : buffer.getOffsets()){
        std::cout << of << ",";
    }
    std::cout << std::endl;

     */
}



bool TestBuffer2() {
    std::cout << "TestBuffer2\n";
    SerializableBoundedQueue buffer((2*sizeof(size_t)+1) * 5);
    size_t prevPrev = 0;
    size_t prev = 0;
    for (char i = 0; i < 4; i++) {
        char c = 'a' + i;
        buffer.push_back(&c, 1);
        auto ii = static_cast<size_t>(i + 1);
        ASSERT(buffer.size(), ii);
        ASSERT(buffer.head()[0], c);
        ASSERT(buffer.getNextPointerPeek(0), prev);
        prev = buffer.getOffsets().at(buffer.getOffsets().size() - 1);
    }
    SerializableBoundedQueue buffer2(buffer.serialize(), buffer.serializationSize());
    ASSERT(buffer2.size(), buffer.size());
    return true;
}


bool TestAES() {

    static struct {
        int32 keylen;
        unsigned char key[32], iv[16], pt[16], ct[16];
    } tests[] = {
            {16,
                    {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7,
                                                                                 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c},
                    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
                            0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
                    {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d,
                            0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a},
                    {0x76, 0x49, 0xab, 0xac, 0x81, 0x19, 0xb2, 0x46, 0xce, 0xe9,
                            0x8e, 0x9b, 0x12, 0xe9, 0x19, 0x7d}},
            {
             24,
                    {0x8e, 0x73, 0xb0, 0xf7, 0xda, 0x0e, 0x64, 0x52, 0xc8, 0x10,
                                                                                 0xf3, 0x2b, 0x80, 0x90, 0x79, 0xe5, 0x62, 0xf8, 0xea, 0xd2,
                                                                                                                                             0x52, 0x2c, 0x6b, 0x7b},
                    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
                            0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
                    {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d,
                            0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a},
                    {0x4f, 0x02, 0x1d, 0xb2, 0x43, 0xbc, 0x63, 0x3d, 0x71, 0x78,
                            0x18, 0x3a, 0x9f, 0xa0, 0x71, 0xe8}
            },
            {
             32,
                    {0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae,
                                                                                       0xf0, 0x85, 0x7d, 0x77, 0x81, 0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61,
                                                                                                                                                         0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4},
                    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
                            0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
                    {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d,
                            0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a},
                    {0xf5, 0x8c, 0x4c, 0x04, 0xd6, 0xe5, 0xf1, 0xba, 0x77, 0x9e,
                            0xab, 0xfb, 0x5f, 0x7b, 0xfb, 0xd6}
            }
    };
    {
        int i = 0;
        uint32_t outLen;
        unsigned char *cipher = AES_Wrapper<16>::encryptCBC(tests[i].key, tests[i].pt, 16, &outLen);
        unsigned char *plain = AES_Wrapper<16>::decryptCBC(tests[i].key, cipher, 16);
        ASSERT(memcmp(plain, tests[i].pt, 16), 0);
        ASSERT(outLen, 16)
        ASSERT(memcmp(cipher, tests[i].ct, 16), 0)
        delete[] cipher;

        ASSERT(memcmp(plain, tests[i].pt, 16), 0)
        delete[] plain;
    }
    {
        int i = 1;
        uint32_t outLen;
        AES_Wrapper<24> aes;
        unsigned char *cipher = AES_Wrapper<24>::encryptCBC(tests[i].key, tests[i].pt, 16, &outLen);

        ASSERT(outLen, 16)
        ASSERT(memcmp(cipher, tests[i].ct, 16), 0)
        delete[] cipher;

        unsigned char *plain = AES_Wrapper<24>::decryptCBC(tests[i].key, tests[i].ct, 16);
        ASSERT(memcmp(plain, tests[i].pt, 16), 0)
        delete[] plain;
    }
    {
        int i = 2;
        uint32_t outLen;
        AES_Wrapper<32> aes;
        unsigned char *cipher = AES_Wrapper<32>::encryptCBC(tests[i].key, tests[i].pt, 16, &outLen);

        ASSERT(outLen, 16)
        ASSERT(memcmp(cipher, tests[i].ct, 16), 0)
        delete[] cipher;

        unsigned char *plain = AES_Wrapper<32>::decryptCBC(tests[i].key, tests[i].ct, 16);
        ASSERT(memcmp(plain, tests[i].pt, 16), 0)
        delete[] plain;
    }

    return true;
}


bool TestAESSigner() {

    static struct {
        int32 keylen;
        unsigned char key[32], iv[16], pt[16], ct[16];
    } tests = {
            32,
            {0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae,
             0xf0, 0x85, 0x7d, 0x77, 0x81, 0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61,
             0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4},
            {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
             0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
            {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d,
             0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a},
            {0xf5, 0x8c, 0x4c, 0x04, 0xd6, 0xe5, 0xf1, 0xba, 0x77, 0x9e,
             0xab, 0xfb, 0x5f, 0x7b, 0xfb, 0xd6}
    };

    std::string from("device 1");
    MemBuffer fromBuf(from.c_str(), from.size());
    AES_SEALER a(&fromBuf);
    a.SEED(tests.key);

    std::string content("abcdefghijklmnop");

    MemBuffer outEnc;
    MemBuffer outVer;
    ASSERT(a.seal(from.c_str(), from.size(), content.c_str(), content.size(), &outEnc), true);
    ASSERT(a.unseal(from.c_str(), from.size(), outEnc.data(), outEnc.size(), &outVer), true);

    ASSERT(outVer.size(), content.size());
    ASSERT(memcmp(content.c_str(), outVer.data(), content.size()), 0);
    return true;
}


bool TestSuite1(){
    std::cout << "Start suite 1\n";
    try {
        MemBuffer id("Id", 2);
        BlockchainApplicationTest<TEST_HASH_SIZE> app(&id);
        BlockchainApplication<TEST_HASH_SIZE>* pApp = &app;
        ChainGenerator chainGenerator = generateChain2;
        return
                TestAESSigner() &&
                TestAES() &&
                TestBuffer() &&
                TestBuffer2() &&
                TestCircularBuffer() &&
                TestPartialBlockGeneration(pApp, chainGenerator, true) &&
                receiveChainTest(pApp, chainGenerator) &&
                BroadcastTest() &&
                CIoTA_TestMain() &&
                CIoTA_TestAgent() &&
                JumpTableTest();
    }
    catch (...){
        return false;
    }
}
