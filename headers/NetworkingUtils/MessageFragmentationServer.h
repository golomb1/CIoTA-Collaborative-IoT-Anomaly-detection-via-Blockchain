//
// Created by golombt on 20/12/2017.
//

#ifndef NETWORKING_UTILS_MESSAGE_FRAGMENTATION_SERVER_H
#define NETWORKING_UTILS_MESSAGE_FRAGMENTATION_SERVER_H


#include <cstdio>
#include <cstring>
#include <random>
#include <map>
#include <functional>
#include <iostream>
#include "CompressionLibrary.h"
#include "BroadcastServer.h"
#include "../CppUtils/CryptoLibrary.h"
#include "../CppUtils/Logger.h"

typedef struct MessageFragmentHeader{
public:
    const bool    _isHeader;
    unsigned char _type;
    unsigned int  _id;
    size_t        _len;
    char          _sha[SHA256_OUTPUT_SIZE];

    MessageFragmentHeader(unsigned char type, unsigned int id, size_t len, char sha[SHA256_OUTPUT_SIZE]) :
            _isHeader(true), _type(type), _id(id), _len(len), _sha()
    {
        memcpy(_sha, sha, SHA256_OUTPUT_SIZE);
    }

} MessageFragmentHeader;

typedef struct FragmentHeader{
    const bool    _isHeader;
    unsigned char _fragId;
    unsigned int  _id;

    FragmentHeader(unsigned char fragId, unsigned int id) :
            _isHeader(false), _fragId(fragId), _id(id) {}
} FragmentHeader;


typedef struct FragmentedPacket{
    unsigned char _type;
    size_t        _len;
    char          _from[IP_ADDRESS_STRING_LENGTH]{};
    //unsigned char _nextFragId;
    size_t        _totalSize{0};
    char          _sha[SHA256_OUTPUT_SIZE];
    std::map<unsigned char, MemBuffer> _mapData{};

    FragmentedPacket(unsigned char type, size_t len, char from[IP_ADDRESS_STRING_LENGTH], char sha[SHA256_OUTPUT_SIZE]) :
            _type(type), _len(len), _sha{}
    {
        memcpy(_from, from, IP_ADDRESS_STRING_LENGTH);
        memcpy(_sha, sha, SHA256_OUTPUT_SIZE);
    }
} FragmentedPacket;


template<typename METADATA>
class MessageFragmentationServer {
private:
    const unsigned int FRAGMENT_SIZE;
    METADATA*  _metadata;
    CompressionLibrary* _library{};
    BroadcastServer* _server{};
    std::map<unsigned char, std::function<bool(METADATA*, const char*, size_t)>>  _handlers{};
    std::map<unsigned int, FragmentedPacket*>  _fragments{};

public:
    MessageFragmentationServer(unsigned int fragmentSize, CompressionLibrary* _library, BroadcastServer* _server)
            : FRAGMENT_SIZE(fragmentSize), _library(_library), _server(_server) {}

    void addHandler(unsigned char type, std::function<bool(METADATA*, const char*, size_t)> handler){
        _handlers.insert(std::make_pair(type, handler));
    }

    void setMetadata(METADATA* metadata){
        _metadata = metadata;
    }

    ~MessageFragmentationServer(){
        cleanFragment();
    }

    void cleanFragment(){
        for(auto pair : _fragments){
            delete pair.second;
        }
        _fragments.clear();
    }

    bool awaitForMessage(){
        return _server->awaitForMessage();
    }


bool receiveFragmentation(){
        char buffer[FRAGMENT_SIZE];
        char fromAddress[IP_ADDRESS_STRING_LENGTH];
        int sizeReceived = _server->recv(buffer, FRAGMENT_SIZE, fromAddress);
        // check which type is it
        if(sizeReceived < sizeof(bool)){
            return false;
        }
        else{
            if(*((bool*)buffer)){
                // this is MessageFragmentHeader
                if(sizeReceived < sizeof(MessageFragmentHeader)){
                    return false;
                }
                auto * header = (MessageFragmentHeader*)buffer;
                if(header->_len == sizeReceived - sizeof(MessageFragmentHeader)){
                    // we got the whole message.
                    if(_handlers.count(header->_type) == 0){
                        return false;
                    }
                    else {


                        SHA256_FUNC func;
                        char sha[SHA256_OUTPUT_SIZE];
                        func.hash(sha, buffer + sizeof(MessageFragmentHeader), header->_len);
                        if (memcmp(sha, header->_sha, SHA256_OUTPUT_SIZE) != 0) {
                            LOG_TRACE_2("Receives defective packet {0}", header->_id);
                            return false;
                        } else {
                            bool shouldFree = false;
                            size_t messageLen;
                            char *message = _library->getDecompressedData(buffer + sizeof(MessageFragmentHeader),
                                                                          header->_len, &messageLen, &shouldFree);

                            char *dataToHandle = message;
                            size_t sizeOfDataToHandle = messageLen;
                            if (dataToHandle == nullptr) {
                                dataToHandle = buffer + sizeof(MessageFragmentHeader);
                                sizeOfDataToHandle = header->_len;
                                shouldFree = false;
                            }
                            bool res = _handlers.at(header->_type)(_metadata, dataToHandle, sizeOfDataToHandle);
                            if (shouldFree) {
                                free(message);
                            }
                            return res;
                        }
                    }
                }
                else{
                    // this is fragmented message.
                    FragmentedPacket* packet = new FragmentedPacket{
                            header->_type, header->_len,
                            fromAddress, header->_sha
                    };
                    packet->_mapData.insert(std::make_pair(0,MemBuffer(buffer + sizeof(MessageFragmentHeader),
                                                                       sizeReceived - sizeof(MessageFragmentHeader))));
                    packet->_totalSize = sizeReceived - sizeof(MessageFragmentHeader);
                    //packet->_nextFragId = 1;
                    _fragments.insert(std::make_pair(header->_id, packet));
                    return false;
                }
            }
            else{
                // this is FragmentHeader
                if(sizeReceived < sizeof(FragmentHeader)){
                    return false;
                }
                else {
                    auto * header = (FragmentHeader*)buffer;
                    // check if we know this id
                    if (_fragments.count(header->_id) == 0) {
                        return false;
                    }
                    else{
                        FragmentedPacket* packet = _fragments.at(header->_id);
                        //if(packet->_nextFragId == header->_fragId){
                        packet->_mapData.insert(std::make_pair(header->_fragId,
                                                               MemBuffer(buffer + sizeof(FragmentHeader),
                                                                         sizeReceived - sizeof(FragmentHeader))));
                        packet->_totalSize += sizeReceived - sizeof(FragmentHeader);
                        //packet->_nextFragId++;
			            LOG_TRACE_5("RECEIVE PACKET {0:d}:{1:d}({2:d}|{3:d})",
                                    header->_id, (int)header->_fragId, packet->_totalSize, packet->_len)
                        if(packet->_len != packet->_totalSize){
                            return false;
                        }
                        else{
                            // we received the whole message
                            bool shouldFree = false;
                            size_t messageLen;
                            MemBuffer packetBuffer;
                            for(auto& p : packet->_mapData){
                                packetBuffer.append(p.second.data(), p.second.size());
                            }

                            SHA256_FUNC func;
                            char sha[SHA256_OUTPUT_SIZE];
                            func.hash(sha, packetBuffer.data(), packetBuffer.size());
                            if(memcmp(sha, packet->_sha, SHA256_OUTPUT_SIZE) != 0){
                                LOG_TRACE_2("Receives defective packet {0}", header->_id);
                                _fragments.erase(header->_id);
                                delete packet;
                                return false;
                            }
                            else {
                                char *message = _library->getDecompressedData(packetBuffer.data(),
                                                                              packetBuffer.size(), &messageLen,
                                                                              &shouldFree);
                                char *dataToHandle = message;
                                size_t sizeOfDataToHandle = messageLen;
                                if (dataToHandle == nullptr) {
                                    dataToHandle = packetBuffer.data();
                                    sizeOfDataToHandle = packetBuffer.size();
                                    shouldFree = false;
                                }
                                bool res = _handlers.at(packet->_type)(_metadata, dataToHandle, sizeOfDataToHandle);
                                if (shouldFree) {
                                    free(message);
                                }
                                _fragments.erase(header->_id);
                                delete packet;
                                return res;
                            }
                        }
                        /*}
                        else{
                            // save the packet for later
                            _fragments.erase(header->_id);
                            delete packet;

                            return false;
                        }*/
                    }
                }
            }
        }
    }



    void broadcastFragmentation(const unsigned short port, unsigned char type, const char *rawData, size_t rawLen)
    {
        bool shouldFree;
        char buffer[FRAGMENT_SIZE];
        size_t sizeOfDataToSend;
        char *compressedData = _library->getCompressedData(rawData, rawLen, &sizeOfDataToSend, &shouldFree);
        const char *dataToSend = compressedData;
        if (compressedData == nullptr) {
            dataToSend = rawData;
            sizeOfDataToSend = rawLen;
        }
        // generate message id
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(10000000);
        unsigned int id = static_cast<unsigned int>(dis(gen));

        char sha[SHA256_OUTPUT_SIZE];
        SHA256_FUNC func;
        func.hash(sha, dataToSend, sizeOfDataToSend);

        MessageFragmentHeader header{type, id, sizeOfDataToSend, sha};
        memcpy(buffer, &header, sizeof(MessageFragmentHeader));

        // is one packet is enough?
        if (sizeOfDataToSend + sizeof(int) > FRAGMENT_SIZE) {
            memcpy(buffer + sizeof(MessageFragmentHeader), dataToSend, FRAGMENT_SIZE - sizeof(MessageFragmentHeader));
            _server->broadcast(port, buffer, FRAGMENT_SIZE );
            sizeOfDataToSend -= (FRAGMENT_SIZE - sizeof(MessageFragmentHeader));
            size_t offset =  (FRAGMENT_SIZE - sizeof(MessageFragmentHeader));
            for (unsigned char i = 1; sizeOfDataToSend > 0; i++) {
                FragmentHeader fragmentHeader{i, id};
                memcpy(buffer, &fragmentHeader, sizeof(FragmentHeader));
                if (sizeOfDataToSend + sizeof(FragmentHeader) >= FRAGMENT_SIZE) {
                    memcpy(buffer + sizeof(FragmentHeader), dataToSend + offset,
                           FRAGMENT_SIZE - sizeof(FragmentHeader));
                    _server->broadcast(port, buffer, FRAGMENT_SIZE);
                    sizeOfDataToSend -= (FRAGMENT_SIZE - sizeof(FragmentHeader));
                    offset += (FRAGMENT_SIZE - sizeof(FragmentHeader));
                }
                else{
                    memcpy(buffer + sizeof(FragmentHeader), dataToSend + offset, sizeOfDataToSend);
                    _server->broadcast(port, buffer, sizeof(FragmentHeader) + sizeOfDataToSend);
                    offset += sizeOfDataToSend;
                    sizeOfDataToSend = 0;
                }
            }

        }
        else{
            memcpy(buffer + sizeof(MessageFragmentHeader), dataToSend, sizeOfDataToSend);
            _server->broadcast(port, buffer, sizeOfDataToSend + sizeof(MessageFragmentHeader));
        }

        if(shouldFree){
            delete[] compressedData;
        }
    }
};


#endif //NETWORKING_UTILS_MESSAGE_FRAGMENTATION_SERVER_H
