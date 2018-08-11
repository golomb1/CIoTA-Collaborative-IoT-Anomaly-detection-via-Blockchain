//
// Created by master on 16/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_EXTENDED_ANOMALY_LISTENER_H
#define CIOTA_COMPLETE_PROJECT_EXTENDED_ANOMALY_LISTENER_H

#include <map>
#include "AnomalyDetectionModel.h"
#include "../NetworkingUtils/SimpleTcpProtocol/SimpleTcpProtocolClient.h"
#include "../CppUtils/Logger.h"


#define REPORT_ANOMALY     1
#define REQUEST_MEMBERSHIP 2


/**
 *  @file    ExtendedAnomalyListener.h
 *  @author  Tomer Golomb (golombt)
 *  @version 1.0
 *
 *  @brief defines an interface for AnomalyListener class.
 *  ExtendedAnomalyListener is an anomaly listener that extend the functionalities of AnomalyListener
 *  to include interaction with a command and control server (C&C) that instruct how to operate when an alert arose.
 *  The additional functionalities of ExtendedAnomalyListener are :
 *
 *  GetMembership return a unique identifier which allows the identification of this instance by the C&C server.
 *  Based on the C&C implementation this can be either unique per device or per instance.
 *
 *  Policies - whenever anomaly alert arises, this listener sends notification to the C&C server and request the
 *  appropriate action that needed to be executed.
 *  These appropriate actions are implemented as Policies and stored as a key-value map where each policy
 *  as a unique identifier that represents it.
 *
 *  Implementation of these policies can be found in the policies folder.
 *  Immediate policies are policies that require fast handling, for example,
 *  for anomaly score less then 20 we want to shut down the application, without waiting for the response of the server.
 *
 */

typedef void(*Policy)();


class ExtendedAnomalyListener : public AnomalyListener {
public:

    /**
     * Constructor for ExtendedAnomalyListener that uses C&C (command & control)
     * server to take action against alerts.
     * @param server is the address of the C&C server.
     * @param port is the port that the C&C server uses.
     */
    ExtendedAnomalyListener(std::string &server, unsigned short port);

    /**
     * @return a membership that was given to this instance by the C&C server.
     */
    MemBuffer* getMembership();

    /**
     * Report on anomaly score.
     * 100 is zero anomaly.
     * 0   is full anomaly.
     * @param score of the anomaly event.
     */
    void anomalyAlert(uint64_t score) override;

    /**
     * Add new policy.
     * @param key is the policy identifier.
     * @param policy is the policy.
     */
    void addPreventionPolicy(int key, Policy policy);

    /**
     * add new immediate policy, the policy will be executed whenever the anomaly score will be less than threshold.
     * @param threshold for the immediate policy.
     * @param key of the policy to use.
     */
    void addImmediatePolicy(uint64_t threshold, int key);

private:
    MemBuffer _membership;
    unsigned short _controlPort;
    std::string    _controlServer;
    SimpleTcpProtocolClient _socket;
    std::map<int, Policy>   _policies;
    std::map<uint64_t, int> _immediatePolicies;
};

#endif //CIOTA_COMPLETE_PROJECT_EXTENDED_ANOMALY_LISTENER_H
