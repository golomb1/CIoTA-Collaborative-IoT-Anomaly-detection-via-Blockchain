//
// Created by master on 16/01/18.
//

#include "../../headers/AnomalyDetection/ExtendedAnomalyListener.h"


ExtendedAnomalyListener::ExtendedAnomalyListener(std::string &server, unsigned short port) :
        _socket(), _controlServer(server), _controlPort(port)
{
    getMembership();
}


MemBuffer* ExtendedAnomalyListener::getMembership() {
    if(_membership.empty()) {
        _socket.startSession(_controlServer.c_str(), _controlPort);
        // report score
        size_t responseLen;
        char message = 1;
        char *response = _socket.send(REQUEST_MEMBERSHIP, &message, sizeof(char), &responseLen);
	_socket.endSession();
        if (responseLen > 0) {
            _membership.clear();
            _membership.append(response, responseLen);
        }
    }
    return &_membership;
}


void ExtendedAnomalyListener::anomalyAlert(uint64_t score) {
    LOG_TRACE_2("anomalyAlert {0}", score)
    // search for immediate policy
    auto prev = _immediatePolicies.rbegin();
    for (auto it = prev; it != _immediatePolicies.rend(); ++it) {
        it++;
        if (prev->first > score && (it == _immediatePolicies.rend() || it->first <= score)) {
            _policies.at(prev->second)();
            return;
        }
        prev++;
    }
    // no immediate policy, ask server for policy
    size_t responseLen;
    _socket.startSession(_controlServer.c_str(), _controlPort);
    // report score
    char *response = _socket.send(REPORT_ANOMALY, &score, sizeof(uint64_t), &responseLen);
    _socket.endSession();
    if (responseLen == sizeof(int)) {
        // get policy
      	 LOG_TRACE_2("anomalyAlert selected policy: {0}", *(int *) response)
        if (_policies.count((*(int *) response)) > 0) {
            // active policy
            _policies.at((*(int *) response))();
        }
    }
    else{
	 LOG_TRACE_2("anomalyAlert unformatted policy.")
    }
    delete[] response;
}

void ExtendedAnomalyListener::addPreventionPolicy(int key, Policy policy) {
    _policies.insert(std::make_pair(key, policy));
}

void ExtendedAnomalyListener::addImmediatePolicy(uint64_t threshold, int key) {
    _immediatePolicies.insert(std::make_pair(threshold, key));
}

