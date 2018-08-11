//
// Created by golombt on 17/12/2017.
//

#include "../../headers/Blockchain/BlockRecord.h"

BlockRecord::BlockRecord(BlockRecord *other) : MemBuffer(other) {}

BlockRecord::BlockRecord(const char *buffer, size_t len) : MemBuffer(buffer, len) {}


BlockRecord::BlockRecord(const char *from, size_t fromLength,
                         const char *content, size_t contentLength) : MemBuffer(nullptr,0)
{
    reserveMore(sizeof(BlockRecordHeader) + fromLength + contentLength);
    BlockRecordHeader header{fromLength, contentLength};
    append(&header, sizeof(BlockRecordHeader));
    append(from, fromLength);
    append(content, contentLength);
}

size_t BlockRecord::fromLength() {
    return (size_t) (*data());
}

size_t BlockRecord::contentLength() {
    return (size_t)(*(data() + 2*sizeof(size_t)));
}

const char *BlockRecord::from() {
    return data() + 3*sizeof(size_t);
}

const char *BlockRecord::content() {
    return from() + fromLength();
}
