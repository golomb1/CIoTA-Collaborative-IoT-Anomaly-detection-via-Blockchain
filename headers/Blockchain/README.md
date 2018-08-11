# Blockchain Implementation

This folder contains the code for a lightweight blockchain implementation.
This code depends on the following classes:
* CppUtils/MemBuffer.h
* CppUtils/Exceptions.h
* CppUtils/Cursor.h
* CppUtils/SerializableBoundedQueue.h


It contains the following files and dependencies:

File | Description | Dependency
---- | ----------- | ----------
BlockRecord.h | Define utilities to create, edit and manage blocks's records. | None.
BlockRecord.cpp | Include implementation of the "BlockRecord.h" file. | BlockRecord.h
Block.h | Define utilities to create, edit and manage blocks | BlockRecord.h
BlockchainApplication.h | Define an interface that uses to configure the blockchain behaviours. | Block.h
Blockchain.h | Defines and implements a generic blockchain manager class. | BlockchainApplication.h

# Blockchain structure
The blockchain implementation is not the simplest, yet it allows for reducing the memory allocated during the execution.
This implementation implements both records and blocks as serialized buffers of bytes to allows straightforwardly sending them,
merely send the buffer, without a need to "duplicate" it.
All blocks operations change the serialized form of the block to maintain its integrity.
The interface of "BlockchainApplication.h" allows to customize the blockchain's properties and therefore results in a more flexible implementation.
To use this module, you need to implement the BlockchainApplication interface and initialize an object of the Blockchain class (defined in "Blockchain.h").

## Record
Each record is composed from two fields: <From><Content>
 * Where <From> is the record owner (the one that created it).
 * Where <Content> is the record content (the one that created it).
For example, in bitcoin <From> is the public key of the one performing the transaction and <Content> is the transaction (The receiver and how much money to transfer).

## Block
Block is a buffer that represents a blockchain block.
Each block is composed from the following fields (in that order):

Field | type | setter | getter | Description
----- | ---- | ------ | ------ | -----------
Block id | size_t | setBlockId | BLOCK_ID | The block position inside the blockchain.
Timestamps | time_t | None | BLOCK_TIMESTAMP | The block's creation time.
Previous Hash | char[H] | setBlockPrevHash | BLOCK_PREV_HASH | The hash of the previous block in tha chain.
Block Seed Size | size_t | None | BLOCK_SEED_SIZE | The size of the seed buffer.
Block Seed | char* | None | BLOCK_SEED | The seed of this block.
Number Of Records | size_t | addRecordContent | BLOCK_NUM_OF_RECORDS | The number of records inside this block.
Block data | char* | None | BLOCK_DATA, getBlockRecord | the list of records.


Additional utilities functions:
* createNewBlock - create a new block structure.
* blockRecordsBegin - return an iterator on the block content.
* blockRecordsEnd - return an end iterator on the block content (when to stop).
* hasRecordFrom - return whether a block contains a record from a given participant.