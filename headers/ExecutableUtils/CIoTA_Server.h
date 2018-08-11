//
// Created by master on 08/02/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_CIOTA_CONTROL_H
#define CIOTA_COMPLETE_PROJECT_CIOTA_CONTROL_H


#include <iostream>
#include <headers/CppUtils/INIReader.h>
#include <headers/CppUtils/AES_Wrapper.h>
#include <headers/JumpTracer/JumpTracer.h>
#include <headers/CIoTA/runControlServer.h>
#include <headers/Blockchain/BlockchainListener.h>
#include <headers/JumpTracer/JumpTracerUtilities.h>
#include <headers/AnomalyDetection/ExtendedAnomalyControlServer.h>

void IgnorePolicy();

void killProcessPolicy();


bool runConfigurableServer(const char* configurationPath,
                           std::function<void(void)> condition,
                           BlockchainListener* blockchainListener,
                           CIoTA::ObserverListener<JumpTracer>* listener,
                           ExtendedAnomalyControlServerListener* control);


#endif //CIOTA_COMPLETE_PROJECT_CIOTA_CONTROL_H
