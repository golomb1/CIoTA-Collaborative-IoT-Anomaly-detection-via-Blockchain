//
// Created by master on 23/01/18.
//

#include <headers/ExecutableUtils/CIoTA_Agent.h>
#include <stdio.h>


pid_t processId(pid_t initialValue = -1) {
    static pid_t theValue = -1;
    if (initialValue != -1 && theValue == -1) {
        theValue = initialValue;
    }
    return theValue;
}


void IgnorePolicy(){
  int fd = open("/tmp/pibrellaFifo", O_WRONLY);
  if(fd >= 0){
      char buffer = 'r';
      write(fd, &buffer, sizeof(char));
      close(fd);
  }
  fd = open("/tmp/pibrellaFifo", O_WRONLY);
  if(fd >= 0){
      char buffer = 'a';
      write(fd, &buffer, sizeof(char));
      close(fd);
  }
  remove("/home/pi/flag.txt");
  GLOBAL_ALERT_WAS_SEND = false;
}

void killProcessPolicy() {
    IgnorePolicy();
    pid_t pid = processId();
    kill(pid, SIGKILL);
    waitpid(-1, nullptr, 0);
    exit(EXIT_FAILURE);
}


bool runConfigurableAgent(const char* configurationPath,
                          BlockchainListener* blockchainListener,
                          std::function<void(void)> trainCallback)
{
    INIReader reader(configurationPath);
    if (reader.ParseError() < 0) {
        return false;
    }

    std::string app = reader.Get("Tracer", "application", "");
    CodeAddress mask =       reader.GetU64Integer("Tracer", "mask", std::numeric_limits<CodeAddress>::max());
    CodeAddress filterFrom = reader.GetU64Integer("Tracer", "filterFrom", 0);
    CodeAddress filterTo   = reader.GetU64Integer("Tracer", "filterTo", std::numeric_limits<CodeAddress>::max());
    size_t window    = reader.GetUInteger("Tracer", "windows", 10*1000);
    size_t threshold = reader.GetUInteger("Tracer", "threshold", 50);
    unsigned int trainInterval = static_cast<unsigned int>(reader.GetUInteger("Tracer", "trainInterval", 30 * 60));


    std::string group   = reader.Get("CIoTA", "multicastGroup", "");
    unsigned short port = static_cast<unsigned short>(reader.GetUInteger("CIoTA", "port", 0));
    std::string controlServerAddress = reader.Get("CIoTA", "controlIP", "");
    unsigned short controlServerPort = static_cast<unsigned short>(reader.GetUInteger("CIoTA", "controlPort", 0));

    size_t pConsensus    = reader.GetUInteger("CIoTA", "pConsensus", std::numeric_limits<size_t>::max());
    uint shareInterval   = static_cast<uint>(reader.GetUInteger("CIoTA", "shareInterval", std::numeric_limits<uint>::max()));
    uint receiveInterval = static_cast<uint>(reader.GetUInteger("CIoTA", "receiveInterval", std::numeric_limits<uint>::max()));
    size_t alpha             = reader.GetUInteger("CIoTA", "alpha", 2);
    size_t completeBlockSize = reader.GetUInteger("CIoTA", "completeBlockSize", std::numeric_limits<size_t>::max());

    size_t capacity     = reader.GetUInteger("CIoTA", "capacity", std::numeric_limits<size_t>::max());
    uint fragmentSize = static_cast<uint>(reader.GetUInteger("CIoTA", "fragmentSize", std::numeric_limits<uint>::max()));

    std::string id        = reader.Get("CIoTA", "EntityKey", "");

    MemBuffer identificationBuffer(&id);
    AES_SEALER sealer(&identificationBuffer);
    SHA256_FUNC hash{};
    CryptoLibrary<SHA256_OUTPUT_SIZE> cryptoLibrary(&hash, &sealer);
    MockCompressionLibrary compressionLibrary;

    CIoTA::AgentParameterPack pack{
            group,                  //std::string & CIoTAGroup;
            port,                   //unsigned short CIoTAPort;
            controlServerAddress,   //std::string &serverAddress;
            controlServerPort,      //unsigned short serverPort;
            {
                    {1, IgnorePolicy},
                    {2, killProcessPolicy},
            },                      //std::map<int, Policy> policies;
            alpha,                  //size_t alpha;
            completeBlockSize,      //size_t completeBlockSize;
            capacity,               //size_t capacity;
            fragmentSize,           //unsigned int fragmentSize;
            shareInterval,          //unsigned int shareInterval;
            receiveInterval,        //unsigned int receiveInterval;
    };


    pid_t pid = fork();
    if (pid == -1) {
        throw SystemError("Failed during fork");
    }
    if (pid == 0) {
        char path[1024];
        const char *appPath = app.c_str();
        strcpy(path, appPath);
        char *const args[] = {path, nullptr};
        if(execve(app.c_str(), args, nullptr) < 0){
            LOG_TRACE_1("FAILED to execute the target program.");
            kill(getppid(), SIGKILL);
        }
        return true;
    } else {
        processId(pid);
        JumpTracer emptyModel(window, mask, filterFrom, filterTo, threshold);
        JumpTracerUtilities utilities(pid, pConsensus, completeBlockSize, trainInterval, &emptyModel, recordJump);
        utilities.setTrainingCallback(std::move(trainCallback));
        ThreadBarrier flag;
        std::thread t(CIoTA::startAgent<JumpTracer, SHA256_OUTPUT_SIZE>, &pack,
                      &utilities, blockchainListener, &cryptoLibrary, &compressionLibrary, &flag);
        waitForInput();
        flag.notify();
        t.join();
        return true;
    }
}


