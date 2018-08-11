//
// Created by tomer on 20/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_EXTENDED_ANOMALY_CONTROL_SERVER_H
#define CIOTA_COMPLETE_PROJECT_EXTENDED_ANOMALY_CONTROL_SERVER_H

#include <map>
#include <string>
#include <netinet/in.h>
#include "../NetworkingUtils/SimpleTcpProtocol/SimpleTcpProtocolServer.h"
#include "../CppUtils/MemBuffer.h"


#define REPORT_ANOMALY     1
#define REQUEST_MEMBERSHIP 2

/**
 * Higher level interface to handle events.
 */
class ExtendedAnomalyControlServerListener{
public:
    /**
     * Select a policy to use for the given score.
     * @param identifier is the identifier of the member that sent the request.
     * @param ip of the member that sent the request.
     * @param score of the event.
     * @param policies is a map of the possible policies that can be taken.
     * @return the key of the policies.
     */
    virtual int selectActionPolicy(const std::string & identifier,
                                   char ip[INET_ADDRSTRLEN],
                                   uint64_t score,
                                   std::map<int, std::string>& policies) = 0;

    /**
     * @param ip is the ip of the requesting device.
     * @return true if the device is accepted into the network or not.
     */
    virtual bool acceptNewMember(char ip[INET_ADDRSTRLEN]) = 0;
};


/**
 * Class that implemented a call back over tcp of a control server.
 */
class ExtendedAnomalyControlServer {
public:

    /**
     * Constructor for the server.
     * @param port that used by this server.
     */
    explicit ExtendedAnomalyControlServer(ExtendedAnomalyControlServerListener* listener, unsigned short port);

    /**
     * Add new policy to this server.
     * @param policyKey
     * @param description
     */
    void addPolicy(int policyKey, std::string& description);


    /**
     * Start this server on a new thread.
     */
    void run();

    /**
     * Stop this server.
     */
    void stop();

    /**
     * A lower implementation that handles the socket request and deligates the relevant data to the acceptNewMember function.
     * If accepted the device receives a unique random identifier.
     * @param address of the client that initiate the connection.
     * @param buffer that the client sent.
     * @param len of the buffer.
     * @param outLen is used to hold the response (return value) size.
     * @return the response for this request (the unique identifier or error).
     */
    static char *handleMembershipRequest(ExtendedAnomalyControlServer* instance, sockaddr_in const *address,
                                         const char *buffer, size_t len, size_t *outLen);


    /**
     * A lower implementation that handles the socket request and deligates the relevant data to the selectActionPolicy function.
     * @param address of the client that initiate the connection.
     * @param buffer that the client sent.
     * @param len of the buffer.
     * @param outLen is used to hold the response (return value) size.
     * @return the response for this request (the policy key).
     */
    static char *handlePolicySelection(ExtendedAnomalyControlServer* instance, sockaddr_in const *address,
                                         const char *buffer, size_t len, size_t *outLen);

protected:
    MemBuffer generateName(char ip[INET_ADDRSTRLEN]);


private:
    unsigned short _port;
    std::map<int, std::string> _policies;
    std::map<std::string, MemBuffer> _members;
    ExtendedAnomalyControlServerListener* _listener;
    SimpleTcpProtocolServer<ExtendedAnomalyControlServer> _socket;
};


#endif //CIOTA_COMPLETE_PROJECT_EXTENDED_ANOMALY_CONTROL_SERVER_H
