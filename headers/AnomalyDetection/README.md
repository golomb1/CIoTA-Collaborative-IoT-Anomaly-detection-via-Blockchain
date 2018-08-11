# Anomaly Detection Utilities

This folder contains the code for interfaces regarding anomaly detection entities.
The entities which defines in this package are:
    * AnomalyDetectionModel
    * AnomalyListener
    * ExtendedAnomalyListener
    * ExtendedAnomalyControlServer

## Anomaly Detection Model
We define AnomalyDetectionModel as an entity that supports three operations:
  
  Operation  | Description
  ---------- | ------------
  Comparison | that return similarity of two models.
  Consume    | give new observation to this model, and the model can either learn from or alert on the observation.
  Serialize  | serialize the model. The serialization is done using the class CppUtils/MemBuffer.h


## Anomaly Listener
AnomalyListener is an entity that can handle anomaly alerts.
Its default implementation is the ExtendedAnomalyListener.

## Extended Anomaly Listener & ExtendedAnomalyControlServer
ExtendedAnomalyListener is an anomaly listener that extend the functionalities of AnomalyListener
to include interaction with a command and control server (C&C) that instruct how to operate when an alert arose.
The additional functionalities of ExtendedAnomalyListener are :
 *  GetMembership return a unique identifier which allows the identification of this instance by the C&C server.
    Based on the C&C implementation this can be either unique per device or per instance.

 *  Policies - whenever anomaly alert arises, this listener sends notification to the C&C server and request the
 appropriate action that needed to be executed.
 These appropriate actions are implemented as Policies and stored as a key-value map where each policy
 as a unique identifier that represents it.
 Implementation of these policies can be found in the policies folder.

 *  Immediate policies are policies that require fast handling, for example,
 for anomaly score less then 20 we want to shut down the application, without waiting for the response of the server.
 
ExtendedAnomalyControlServer is the implementation of the C&C server that expected by the ExtendedAnomalyListener.
It is an abstract class that require the implementation of the following methods:
    
    /**
     * Select a policy to use for the given score.
     * @param identifier is the identifier of the member that sent the request.
     * @param score of the event.
     * @return the key of the policies.
     */
    virtual int selectActionPolicy(std::string identifier, uint64_t score) = 0;

    /**
     * @param ip is the ip of the requesting device.
     * @return true if the device is accepted into the network or not.
     */
    virtual bool acceptNewMember(char ip[INET_ADDRSTRLEN]) = 0;
    
The rest of the communication is implemented over TCP connections.