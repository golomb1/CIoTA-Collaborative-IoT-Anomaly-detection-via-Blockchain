//
// Created by master on 08/02/18.
//
#include <headers/ExecutableUtils/CIoTA_Server.h>



bool runConfigurableServer(const char* configurationPath,
                           std::function<void(void)> condition,
                           BlockchainListener* blockchainListener,
                           CIoTA::ObserverListener<JumpTracer>* listener,
                           ExtendedAnomalyControlServerListener* control)
{
    INIReader reader(configurationPath);
    if (reader.ParseError() < 0) {
        return false;
    }

    std::string group = reader.Get("CIoTA", "multicastGroup", "");
    unsigned short port = static_cast<unsigned short>(reader.GetUInteger("CIoTA", "port", 0));
    std::string controlServerAddress = reader.Get("CIoTA", "controlIP", "");
    unsigned short controlServerPort = static_cast<unsigned short>(reader.GetUInteger("CIoTA", "controlPort", 0));

    uint receiveInterval = static_cast<uint>(reader.GetUInteger("CIoTA", "receiveInterval",
                                                                std::numeric_limits<uint>::max()));
    size_t alpha = reader.GetUInteger("CIoTA", "alpha", 2);
    size_t completeBlockSize = reader.GetUInteger("CIoTA", "completeBlockSize", std::numeric_limits<size_t>::max());

    size_t capacity = reader.GetUInteger("CIoTA", "capacity", std::numeric_limits<size_t>::max());
    uint fragmentSize = static_cast<uint>(reader.GetUInteger("CIoTA", "fragmentSize",
                                                             std::numeric_limits<uint>::max()));

    std::string id = reader.Get("CIoTA", "EntityKey", "");

    MemBuffer identificationBuffer(&id);

    CIoTA::ServerParameterPack pack{
            group,                   //std::string &CIoTAGroup;
            port,                    //unsigned short CIoTAPort;
            controlServerPort,       //unsigned short controlPort;
            {
                    {1, "Ignore"},
                    {2, "Kill the application"},
            },                      //std::map<int, std::string> policies;
            alpha,                  //size_t alpha;
            completeBlockSize,      //size_t completeBlockSize;
            capacity,               //size_t capacity;
            fragmentSize,           //unsigned int fragmentSize;
            receiveInterval         //unsigned int receiveInterval;
    };

    if(blockchainListener!=nullptr){
        blockchainListener->setCompleteBlockSize(completeBlockSize);
    }
    AES_SEALER sealer(&identificationBuffer);
    SHA256_FUNC hash{};
    CryptoLibrary <SHA256_OUTPUT_SIZE> cryptoLibrary(&hash, &sealer);


    MockCompressionLibrary compressionLibrary;
    ThreadBarrier flag;
    std::thread t(CIoTA::startControlServer < JumpTracer, SHA256_OUTPUT_SIZE > ,
                  &pack, &cryptoLibrary, listener, &compressionLibrary,
                  control, blockchainListener, nullptr, &flag);
    condition();
    flag.notify();
    t.join();
}
