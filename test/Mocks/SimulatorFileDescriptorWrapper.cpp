//
// Created by golombt on 28/12/2017.
//

#include <cstring>
#include "SimulatorFileDescriptorWrapper.h"


std::vector<MemBuffer>   SimulatorFileDescriptorWrapper::packets;
std::mutex               SimulatorFileDescriptorWrapper::lock;
std::atomic<int>         SimulatorFileDescriptorWrapper::globalCounter;
std::atomic<bool>        SimulatorFileDescriptorWrapper::alertUnconditionally(false);
std::map<size_t, size_t> SimulatorFileDescriptorWrapper::lastReceivedPacket;


#define __min(x,y) ((x) > (y) ? (y) : (x))

int SimulatorFileDescriptorWrapper::recv(char *buffer, size_t len, char fromAddress[IP_ADDRESS_STRING_LENGTH]) {
    {
        std::unique_lock<std::mutex> lk(SimulatorFileDescriptorWrapper::lock);
        if(SimulatorFileDescriptorWrapper::lastReceivedPacket[_id] < SimulatorFileDescriptorWrapper::packets.size()){
            MemBuffer b = SimulatorFileDescriptorWrapper::packets.at(SimulatorFileDescriptorWrapper::lastReceivedPacket[_id]);
            SimulatorFileDescriptorWrapper::lastReceivedPacket[_id]++;
            memcpy(buffer, b.data(), __min(b.size(), len));
            return static_cast<int>(__min(b.size(), len));
        }
        else{
            return -1;
        }
    }
}

int SimulatorFileDescriptorWrapper::broadcast(unsigned short port, const char *buffer, size_t len) {
    std::unique_lock<std::mutex> lk(SimulatorFileDescriptorWrapper::lock);
    //std::cout << "Device " << _id << " Broadcasted" << std::endl;
    SimulatorFileDescriptorWrapper::packets.emplace_back(buffer, len);
    lk.unlock();
    cv.notify_one();
}

bool SimulatorFileDescriptorWrapper::awaitForMessage() {
    /*std::unique_lock<std::mutex> lk(SimulatorFileDescriptorWrapper::lock);
    //std::cout << "Device " << _id << " wait for message\n";
    cv.wait(lk, [this]{return (lastReceivedPacket < SimulatorFileDescriptorWrapper::packets.size());});*/
    return SimulatorFileDescriptorWrapper::lastReceivedPacket[_id] < SimulatorFileDescriptorWrapper::packets.size();
}