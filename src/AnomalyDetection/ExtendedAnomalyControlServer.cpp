//
// Created by tomer on 20/01/18.
//

#include <arpa/inet.h>
#include <cstring>
#include <random>
#include "../../headers/AnomalyDetection/ExtendedAnomalyControlServer.h"


#define CHAR_MAX_VALUE  255
#define IDENTIFIER_SIZE 128

ExtendedAnomalyControlServer::ExtendedAnomalyControlServer(ExtendedAnomalyControlServerListener* listener,
                                                           unsigned short port)
        : _socket(), _listener(listener), _policies(), _port(port)
{
    _socket.addHandle(REPORT_ANOMALY, &ExtendedAnomalyControlServer::handlePolicySelection);
    _socket.addHandle(REQUEST_MEMBERSHIP, &ExtendedAnomalyControlServer::handleMembershipRequest);
    _socket.setMetadata(this);
}


void ExtendedAnomalyControlServer::run() {
    _socket.run(_port);
}

void ExtendedAnomalyControlServer::stop() {
    _socket.stop();
}

void ExtendedAnomalyControlServer::addPolicy(int policyKey, std::string &description) {
    _policies.insert(std::make_pair(policyKey, description));
}




MemBuffer ExtendedAnomalyControlServer::generateName(char ip[INET_ADDRSTRLEN]) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, CHAR_MAX_VALUE);
    MemBuffer id;
    id.reserveMore(IDENTIFIER_SIZE + INET_ADDRSTRLEN);
    id.append(ip, INET_ADDRSTRLEN);
    for(int i=0; i < IDENTIFIER_SIZE - INET_ADDRSTRLEN; i++){
        id.appendValue(static_cast<char>(dis(gen)), 1);
    }
    return id;
}
char *ExtendedAnomalyControlServer::handleMembershipRequest(ExtendedAnomalyControlServer* instance,
                                                            sockaddr_in const *address,
                                                            const char *buffer, size_t len,
                                                            size_t *outLen)
{
    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(address->sin_addr), str, INET_ADDRSTRLEN);
    if(len == sizeof(char) && buffer[0] == 1){
        if(instance->_members.count(str) == 0 && instance->_listener->acceptNewMember(str)){
            MemBuffer id = instance->generateName(str);
            instance->_members.insert(std::make_pair(str, id));
            auto * retBuffer = new char[id.size()];
            memset(retBuffer, 0, id.size());
            memcpy(retBuffer, id.data(), id.size());
            *outLen = id.size();
            return retBuffer;
        }
        else{
            MemBuffer id = instance->_members.at(str);
            if(true){//instance->_listener->acceptReconnection(id, str)){
            	auto * retBuffer = new char[id.size()];
            	memset(retBuffer, 0, id.size());
            	memcpy(retBuffer, id.data(), id.size());
            	*outLen = id.size();
            	return retBuffer;
	    }
	    else{
            	throw ProtocolException("Request denied");
	    }
        }
    }
    else{
        throw ProtocolException("Invalid Request");
    }
}

char *ExtendedAnomalyControlServer::handlePolicySelection(ExtendedAnomalyControlServer* instance,
                                                          sockaddr_in const *address,
                                                          const char *buffer, size_t len,
                                                          size_t *outLen)
{
    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(address->sin_addr), str, INET_ADDRSTRLEN);
    if(len == sizeof(uint64_t)){
        uint64_t grade = *((uint64_t*)buffer);
        if(instance->_members.count(str) == 1){
            int key = instance->_listener->selectActionPolicy(str, str, grade, instance->_policies);
            auto * retBuffer = new int(key);
            *outLen = sizeof(int);
            return reinterpret_cast<char *>(retBuffer);
        }
        else{
            throw ProtocolException("Unknown Requester");
        }
    }
    else{
        throw ProtocolException("Invalid Request");
    }
}
