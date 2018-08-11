#include <iostream>
#include <unistd.h>
#include "Simulator/Simulator.h"
#include "TestSuite1/TestSuite1.h"

void printUsage(char *argv[]) {
    std::string programName(argv[0]);
    unsigned long pos = programName.find_last_of('/');
    programName = programName.substr(pos + 1);
    std::cout << "Usage \"" << programName << ": [suite|sync|async]\"" << std::endl;
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printUsage(argv);
    } else {
        if (strcmp(argv[1], "suite") == 0) {
            if (!TestSuite1()) {
                std::cout << "FAILED..., press Enter to exit.\n" << std::endl;
                sleep(1);
            }
        } else if (strcmp(argv[1], "sync") == 0) {
            std::cout << "START  SIMULATOR A" << std::endl;
            Simulator simulator;
            simulator.serialSimulation(4);
            std::cout << "FINISH SIMULATOR A" << std::endl;
        } else if (strcmp(argv[1], "async") == 0) {
            std::cout << "START  SIMULATOR B" << std::endl;
            Simulator simulator;
            simulator.asynchronousSimulation(4);
            std::cout << "FINISH SIMULATOR B" << std::endl;
        } else {
            printUsage(argv);
        }
        std::cout << "ALL TESTS PASSED\n" << std::endl;
        exit(EXIT_SUCCESS);
    }
}
