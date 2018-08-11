//
// Created by master on 08/02/18.
//


#include <iostream>
#include <headers/ExecutableUtils/CIoTA_Server.h>
#include <headers/CIoTA/LogObserverListener.h>

#define NUM_OF_MANDATORY_ARGS    1
#define NUM_OF_OPT_ARGS          1
#define OPT_ARG_LOG              2


class CliExtendedAnomalyControlServerListener : public ExtendedAnomalyControlServerListener {
private:

public:
    CliExtendedAnomalyControlServerListener() = default;


    int selectActionPolicy(const std::string& identifier,
                           char ip[INET_ADDRSTRLEN],
                           uint64_t score,
                           std::map<int, std::string>& policies) override
    {
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "Action is needed:" << std::endl;
        std::cout << "Device: " << identifier.c_str() << std::endl;
        std::cout << "IP    : " << ip << std::endl;
        std::cout << "Score : " << score << std::endl;
        std::cout << "Possible actions: " << std::endl;
        for(auto it : policies){
            std::cout << "\t" << it.first << ". " << it.second.c_str() << std::endl;
        }
        std::cout << "Please enter the desired action number: " << std::flush;
        int choice = 0;
        std::cin >> choice;
        return choice;
    }

    bool acceptNewMember(char *ip) override {
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "Action is needed:" << std::endl;
        std::cout << "IP    : " << ip << std::endl;
        std::cout << "Request to join the network" << std::endl;
        std::cout << "Possible actions: " << std::endl;
        std::cout << "\t1. Allows" << std::endl;
        std::cout << "\t2. Deny" << std::endl;
        std::cout << "Please enter the desired action number: " << std::flush;
        int choice = 0;
        std::cin >> choice;
        return choice == 1;
    }
};

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
    LogObserverListener <JumpTracer> *listener = nullptr;
    if (argc > OPT_ARG_LOG) {
        listener = new LogObserverListener<JumpTracer>(argv[OPT_ARG_LOG], 1048576 * 5, 1);
    }
    CliExtendedAnomalyControlServerListener control;
    if(!(runConfigurableServer(argv[1], waitForInput, nullptr, nullptr, &control)))
    {
        delete listener;
        std::cout << "Can't load 'test.ini'\n";
        printUsage(argv[0]);
    }
    delete listener;
    return EXIT_SUCCESS;
}