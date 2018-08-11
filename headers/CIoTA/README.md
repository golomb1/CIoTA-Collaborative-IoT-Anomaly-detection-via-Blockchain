# CIoTA - Core package
This is the implementation of CIoTA components.
In the core folder you can find the following classes:

#### CIoTA Application:
Implemented by both BaseCIoTABlockchainApplication and CIoTABlockchainApplication.
This class implements the BlockchainApplication interface and separate the blockchain execution from the rest of the agent functionalities.
This class defines the functionalities needed by the blockchain itself.
These classes uses ModelUtilities interface as a bridge between the CIoTA functionalities and the model.

* BaseCIoTABlockchainApplication - the base class that only implements the functionalities needed to observe the CIoTA network,
without direct interaction with the network. It is achieved by implementing only the receiving functionalities without the sharing and the model anomaly detection functionalities. 

* CIoTABlockchainApplication - extend the BaseCIoTABlockchainApplication to include both the sharing and anomaly detection model functionalities.
These functionalities are representing by the following functions:

Function | Use | Functionality
-------- | --- | -------------
getSeed  | return a new seed for a new block. | Sharing
makeRecord | create a new record for sharing through the blockchain. | Sharing
hasModelForSharing | return true if the anomaly detection model is ready to be shared in the network. | Sharing
executeModel | Execute the anomaly detection model. | Anomaly detection model

#### Agent
Implemented by both BaseAgent and Agent.
This class is responsible for handling the network propagation, aka the sharing and receiving of notification through the blockchain.
The two classes are as following:
 * BaseAgent - This is a base implementation of CIoTA Agent, 
 which contain the logic shared by Agent and Viewer. 
 Support only viewing and without attempting to change or affect the system.
 Use this class only if this is your aim.
 
 * Agent - extend BaseAgent functionalities to support both sharing and handling of a Anomaly Detection models.
 
# Execution utilities
## Shared structures.
For better usage, we implemented several utilities that ease the execution procedure.
Both Observer and Agent needs the following structures:
* Compression library - used to compressed the model data before sending, 
it is not implemented and you can use the default implementation MockCompressionLibrary or implement one of your own.

* Encryption library to sign and encrypt messages as well as computing hash.
You can initialize instance of the CryptoLibrary class and pass both the hashing and signing utilities to it.
For hashing you can use the default implementation that uses SHA256 in the class SHA256_FUNC.
For singing and verifying you can either use symmetric algorithm (not safe for deployment) or asymmetric.
Symmetric implementation uses AES128 bit in CBC mode (class AES_SEALER).
Asymmetric implementation will use RSA or DSA (to be implemented).


    AES_SEALER sealer{};
    SHA256_FUNC hash{};
    CryptoLibrary<SHA256_OUTPUT_SIZE> cryptoLibrary(&hash, &sealer);
    
* BlockchainListener - is used to allows feedback on the agent situation,
apart from debugging, an agent does not need a blockchain listener.
However, observer might want to supply such listener to show a log on the system state.

* For the command server, additional functionalities are needed, these functionalities are implemented by the interface ObserverListener.
This interface, handle all the request that can be done by a CIoTA agent.
Default implementation is the LogObservationListener that uses the console to log on the network state.


* Model Utilities - used to assert and handle models, observer might not need it if it want to observe the blockchain itself.
In cases where the observer want to observe the consensus model, he can supply the relevant model utilities.
For observation we supply an implementation called "ObserverUtilities" which implements the functionalities 
need by an observer while allowing delegation to a real implementation of ModelUtilities. 


## Observer

The execution procedure of a CIoTA Observer is as following:
1. Create a new compression and encryption library.
2. Initiate a new MulticastServerDescriptor instance.
3. Initiate a new ExtendedAnomalyControlServer instance.
4. Initiate a new BaseCIoTABlockchainApplication.
5. Initiate a new BaseAgent.
6. Execute the agent.

For ease of use we implemented the methods runControlServer and runObserver that implements these steps.

Example (from the MockServer.cpp file)


    AES_SEALER sealer(&id);
    SHA256_FUNC hash{};
    MockCompressionLibrary compressionLibrary;
    LogObserverListener<JumpTracer> observerListener;
    MockExtendedAnomalyControlServerListener listener;
    CryptoLibrary<SHA256_OUTPUT_SIZE> cryptoLibrary(&hash, &sealer);
    CIoTA::startControlServer<JumpTracer, SHA256_OUTPUT_SIZE>(
            &pack,
            &cryptoLibrary,
            &observerListener,
            &compressionLibrary,
            &listener,
            nullptr,
            nullptr
    );

## Agent

The execution procedure of a CIoTA Agent is as following:
1. Create a new compression and encryption library.
2. Create instance of model utilities.
3. Initiate a new MulticastServerDescriptor instance.
3. Initiate a new ExtendedAnomalyListener instance.
4. Initiate a new CIoTABlockchainApplication.
5. Initiate a new Agent.
6. Execute the agent.

For ease of use we implemented the method runAgent that implements these steps.

Example (from the MockAgent.cpp file)

    AES_SEALER sealer(&idBuf);
    SHA256_FUNC hash{};
    MockCompressionLibrary compressionLibrary;
    CryptoLibrary<SHA256_OUTPUT_SIZE> cryptoLibrary(&hash, &sealer);
    CIoTA::startAgent<JumpTracer, SHA256_OUTPUT_SIZE>(
            &pack,
            &utilities,
            nullptr,
            &cryptoLibrary,
            &compressionLibrary
    );
