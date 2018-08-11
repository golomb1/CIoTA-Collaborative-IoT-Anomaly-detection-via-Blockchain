//
// Created by master on 08/02/18.
//agent

#include <iostream>
#include <headers/ExecutableUtils/CIoTA_Agent.h>

#define NUM_OF_MANDATORY_ARGS    1
#define NUM_OF_OPT_ARGS          1

void printUsage(char* argv0){
    std::string programName(argv0);
    unsigned long pos = programName.find_last_of('/');
    programName = programName.substr(pos + 1);
    std::cout << "Usage \"" << programName << " configurationFile [LogFilePath] \"." << std::endl;
    std::cout << "configurationFile is the INI file containing CIoTA configurations." << std::endl;
    std::cout << "LogFilePath is optional argument that specify the log file path." << std::endl;
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (!(argc >= NUM_OF_MANDATORY_ARGS + 1 && argc <= NUM_OF_MANDATORY_ARGS + NUM_OF_OPT_ARGS + 1)) {
        printUsage(argv[0]);
    }
    if(!(runConfigurableAgent(argv[1], nullptr, nullptr)))
    {
        std::cout << "Can't load configuration file.\n";
        printUsage(argv[0]);
    }
}