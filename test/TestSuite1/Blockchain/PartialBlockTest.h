
//
// Created by golombt on 20/12/2017.
//

#ifndef TEST_PARTIAL_BLOCK_BLOCKCHAIN_TEST_H
#define TEST_PARTIAL_BLOCK_BLOCKCHAIN_TEST_H

#include <sstream>
#include <iomanip>
#include "../../TEST.h"
#include "../../Mocks/BlockchainTestUtils.h"




/**
 * @test the partial block generation by:
 *    (1) generating new partial block (when non existing) and without chain.
 *    (2) generating new partial block (when non existing) and with chain.
 *    (3) updating the partial block when without a chain.
 *    (4) updating the partial block when with a chain.
 *    (5) record already in the partial block when without a chain.
 *    (6) record already in the partial block when with a chain.
 * @param app - application instance.
 */
bool TestPartialBlockGeneration(BlockchainApplication<TEST_HASH_SIZE> *app,
                                ChainGenerator chainGenerator, bool checkSeed);


#endif //TEST_PARTIAL_BLOCK_BLOCKCHAIN_TEST_H
