//
// Created by master on 18/01/18.
//


#include <iostream>
#include <QtWidgets/QApplication>
#include <headers/CIoTA/PrinterObserverListener.h>
#include <src/ControlServer/GUI/mainwindow.h>
#include "../headers/CIoTA/runAgent.h"
#include "../headers/CppUtils/AES_Wrapper.h"
#include "../headers/CIoTA/runControlServer.h"
#include "../headers/JumpTracer/JumpTracer.h"

#define NUM_OF_ARGS    1
#define TEST_GROUP     "225.0.0.37"
#define TEST_PORT      12345
#define CONTROL_PORT   12346
#define SERVER_ID      "SERVER"


int main(int argc, char *argv[]) {
    if (argc != NUM_OF_ARGS + 1) {
        std::string programName(argv[0]);
        unsigned long pos = programName.find_last_of('/');
        programName = programName.substr(pos + 1);
        std::cout << "Usage \"" << programName << ": [completeBlockSize]\"." << std::endl;
        std::cout << "completeBlockSize is the number of devices in the mock simulation." << std::endl;
        exit(EXIT_FAILURE);
    }
    size_t completeBlockSize = static_cast<size_t>(std::stoi(argv[1]));
    std::string group(TEST_GROUP);


    CIoTA::ServerParameterPack pack{
            group,                   //std::string &CIoTAGroup;
            TEST_PORT,               //unsigned short CIoTAPort;
            CONTROL_PORT,            //unsigned short controlPort;
            {
                    {1, "Policy1"},
                    {2, "Policy2"},
            },                      //std::map<int, std::string> policies;
            20,                     //size_t alpha;
            completeBlockSize,      //size_t completeBlockSize;
            10 * 1024,                //size_t capacity;
            256,                    //unsigned int fragmentSize;
            5                       //unsigned int receiveInterval;
    };
    MemBuffer id(SERVER_ID, strlen(SERVER_ID));
    AES_SEALER sealer(&id);
    SHA256_FUNC hash{};
    CryptoLibrary<SHA256_OUTPUT_SIZE> cryptoLibrary(&hash, &sealer);

    QApplication a(argc, argv);
    MainWindow w;
    w.setHashSize(SHA256_OUTPUT_SIZE);
    w.setCompleteBlockSize(completeBlockSize);
    w.show();

    PrinterObserverListener<JumpTracer> listener(&w);
    MockCompressionLibrary compressionLibrary;
    ThreadBarrier flag;
    std::thread control(CIoTA::startControlServer<JumpTracer, SHA256_OUTPUT_SIZE>, &pack,
                        &cryptoLibrary, &listener, &compressionLibrary, &w, &w, nullptr, &flag);
    a.exec();
    flag.notify();
    control.join();

}