//
// Created by golombt on 25/12/2017.
//

#ifndef TEST_SIMULATOR_FILE_DESCRIPTOR_WRAPPER_H
#define TEST_SIMULATOR_FILE_DESCRIPTOR_WRAPPER_H


#include <map>
#include "../../headers/CppUtils/MemBuffer.h"
#include "../../headers/AnomalyDetection/AnomalyDetectionModel.h"
#include "../../headers/NetworkingUtils/BroadcastServer.h"
#include <iostream>
#include <condition_variable>
#include <atomic>


class SimulatorFileDescriptorWrapper : public BroadcastServer, public AnomalyListener {
private:

    size_t _id;
    bool _alert;
    std::condition_variable cv;

public:
    static std::atomic<int> globalCounter;
    static std::vector<MemBuffer> packets;
    static std::mutex lock;
    static std::atomic<bool> alertUnconditionally;
    static std::map<size_t, size_t> lastReceivedPacket;

    explicit SimulatorFileDescriptorWrapper(size_t id) : _id(id), _alert(false){
        SimulatorFileDescriptorWrapper::lastReceivedPacket[id] = 0;
    }

    int recv(char *buffer, size_t len, char fromAddress[IP_ADDRESS_STRING_LENGTH]) override;

    int broadcast(unsigned short port, const char *buffer, size_t len) override;

    bool awaitForMessage() override;


    void anomalyAlert(uint64_t score) override {
        std::unique_lock<std::mutex> lk(SimulatorFileDescriptorWrapper::lock);
        if(!_alert){
            _alert = true;
            globalCounter--;
        }
        if(globalCounter == 0){
            //std::cout << "All Device Alerts" << std::endl;
        }
    }
};


#endif //TEST_SIMULATOR_FILE_DESCRIPTOR_WRAPPER_H
