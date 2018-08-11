//
// Created by golombt on 27/12/2017.
//
#include <string>
#include "CIoTA_TestApplication.h"
#include "../Blockchain/PartialBlockTest.h"
#include "../Blockchain/receiveChainTest.h"


/*
BlockRecord *CIoTA_Test_ChainGenerator(int fromIndex,int seedIndex) {
    char from[10];
    char seed[10];
    char cont[10];
    memset(from, 0, 10);
    memset(seed, 0, 10);
    memset(cont, 0, 10);
    strcpy(from, "From");
    from[strlen("From")] = static_cast<char>('0' + fromIndex);
    strcpy(seed, "Seed");
    seed[strlen("Seed")] = static_cast<char>('0' + seedIndex);
    strcpy(cont, "Content");
    cont[strlen("Content")] = static_cast<char>('0' + fromIndex);
    auto * record = new BlockRecord(from, 10, seed, 10, cont, 10);
    return record;
}*/



bool CIoTA_TestMain(){
    std::cout << "CIoTA_TestMain\n";
    MemBuffer id("id", 2);
    EmptySHA256CryptoLibraryTest cryptoLibrary;
    StringModelUtilities utilities;
    ChainGenerator chainGenerator = generateChain2;
    auto * app = new CIoTA::CIoTABlockchainApplication<std::string, SHA256_OUTPUT_SIZE>(&id, 50, 3, nullptr, &cryptoLibrary, &utilities);
    bool res =
            //TestPartialBlockGeneration(app, chainGenerator, false) &&
                    receiveChainTest(app, chainGenerator);
    delete app;
    return res;
}