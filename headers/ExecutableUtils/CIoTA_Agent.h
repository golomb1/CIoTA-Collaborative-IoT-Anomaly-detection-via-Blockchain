//
// Created by master on 08/02/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_CIOTA_AGENT_H
#define CIOTA_COMPLETE_PROJECT_CIOTA_AGENT_H


#include <wait.h>
#include <csignal>
#include <iostream>
#include <headers/JumpTracer/Tracer.h>
#include <headers/CppUtils/INIReader.h>
#include <headers/CppUtils/AES_Wrapper.h>
#include <headers/CIoTA/runControlServer.h>
#include <headers/JumpTracer/JumpTracerUtilities.h>



extern bool GLOBAL_ALERT_WAS_SEND;


void IgnorePolicy();

void killProcessPolicy();


bool runConfigurableAgent(const char* configurationPath,
                          BlockchainListener* blockchainListener,
                          std::function<void(void)> trainCallback);

#endif //CIOTA_COMPLETE_PROJECT_CIOTA_AGENT_H
