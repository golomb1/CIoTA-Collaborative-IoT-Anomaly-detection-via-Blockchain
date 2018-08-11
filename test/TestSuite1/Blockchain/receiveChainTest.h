//
// Created by golombt on 20/12/2017.
//

#ifndef TEST_RECEIVE_CHAIN_TEST_H
#define TEST_RECEIVE_CHAIN_TEST_H


#include <sstream>
#include <iomanip>
#include "../../TEST.h"
#include "../../Mocks/BlockchainTestUtils.h"



/**
 * @test acceptance of chain, partial block grow and entire chain, between 4 participants.
 * @test acceptance of chain from participant without a chain.
 * @test test too many partial blocks.
 * @test test record with wrong seed inside a block.
 * @test test block that includes two record with the same sender (unacceptable block).
 * @test test partial block that includes two record with the same sender (unacceptable partial block).
 * @test test chain which its last block which is not partial includes two record with the same sender (unacceptable block).
 * @test test chain with wrong hash pointer.
 * @test test chain with wrong hash pointer in the partial block.
 * @test chain that start with missing block at the start compare to the current chain.
 * @test collision with longer chain.
 * @param app - application instance.
 */
bool receiveChainTest(BlockchainApplication<TEST_HASH_SIZE>* app,
                      ChainGenerator chainGenerator);


#endif //TEST_RECEIVE_CHAIN_TEST_H
