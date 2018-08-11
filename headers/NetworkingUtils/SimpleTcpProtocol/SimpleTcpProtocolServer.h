//
// Created by master on 16/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_SIMPLE_TCP_PROTOCOL_SERVER_H
#define CIOTA_COMPLETE_PROJECT_SIMPLE_TCP_PROTOCOL_SERVER_H

#include <map>
#include <string>
#include <thread>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include "SimpleTcpProtocol.h"
#include "../../CppUtils/Exceptions.h"
#include "ProtocolException.h"

#define NO_HANDLE_ERROR_MESSAGE "No such handle"
#define MAX_CONNECTIONS         50

template <typename MD>
using MessageHandler = char *(*)(MD* metadata, sockaddr_in const *address, const char *buffer, size_t len, size_t *outLen);

template <typename MD>
class SimpleTcpProtocolServer {
private:
    MD* _metadata;
    int stopPipe[2]{};
    std::thread *_thread;
    std::map<unsigned char, MessageHandler<MD>> _handlers{};

public:

    /**
     * Constructor
     * Build a new Simple protocol server.
     */
    SimpleTcpProtocolServer() : _metadata{nullptr}, _thread(nullptr)
    {
        int ret = pipe(stopPipe);
        if (ret != 0) {
            throw SystemError("SimpleTcpProtocolServer::constructor::pipe failed");
        }
        fcntl(stopPipe[0], F_SETFL, O_NONBLOCK);
    }

    /**
     * Destructor
     * Destroy a Simple protocol server.
     */
    ~SimpleTcpProtocolServer(){
        stop();
        close(stopPipe[0]);
        close(stopPipe[1]);
    }

    /**
     * Add new request handler to this server.
     * @param type of message to handle.
     * @param handler is the function that will handle the message.
     */
    void addHandle(unsigned char type, MessageHandler<MD> handler){
        _handlers.insert(std::make_pair(type, handler));
    }

    /**
     * set the metadata that is passed to the handler functions.
     * @param md is the new metadata used by the handlers.
     */
    void setMetadata(MD* md){
        _metadata = md;
    }

    /**
     * Execute this server on a given port in a new thread.
     * @param port to use for the server.
     */
    void run(unsigned short port) {
        if (_thread == nullptr) {
            char buffer[1024];
            while (read(stopPipe[0], buffer, 1024) >= 0) {}
            _thread = new std::thread(&SimpleTcpProtocolServer::worker, this, port);
        }
    }

    /**
     * Stop the execution of the server, if not running then nothing occurs.
     */
    void stop() {
        char flag = 1;
        if (_thread != nullptr) {
            write(stopPipe[1], &flag, sizeof(char));
            _thread->join();
            _thread = nullptr;
        }
    }

protected:

    /**
     * Worker function that executed on a new thread when invoking run.
     * @param port to listen on.
     */
    void worker(unsigned short port) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            throw SystemError("SimpleTcpProtocolServer::run::socket failed");
        }
        fcntl(sock, F_SETFL, O_NONBLOCK);
        sockaddr_in serverAddress{};
        memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        uint yes = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
            throw SystemError("SimpleTcpProtocolServer::setsockopt SO_REUSEADDR failed");
        }
        if (bind(sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
            close(sock);
            throw SystemError("SimpleTcpProtocolServer::run::bind failed");
        }
        if (listen(sock, MAX_CONNECTIONS) < 0) {
            close(sock);
            throw SystemError("SimpleTcpProtocolServer::run::listen failed");
        }
        while (true) {
            //accept connection from an incoming client
            struct pollfd fds[2];
            memset(fds, 0, sizeof(fds));
            fds[0].fd = sock;
            fds[0].events = POLLIN;
            fds[1].fd = stopPipe[0];
            fds[1].events = POLLIN;

            int status = poll(fds, 2, -1);
            if (status < 0) {
                throw std::system_error(errno, std::system_category(), "poll failed");
            }
            if (status == 0) {
                // time out, ignore.
                continue;
            }
            // check on each event.
            if (fds[1].revents != 0) {
                break;
            }
            if (fds[0].revents != 0) {
                handleSession(sock);
            }
        }
        close(sock);
    }


    /**
     * Handle session and new connection from a client.
     * @param sock is the socket that listen for new conenections.
     */
    void handleSession(int sock){
        size_t clientAddressLen = sizeof(struct sockaddr_in);
        sockaddr_in clientAddress{};
        int client_sock = accept(sock, (struct sockaddr *) &clientAddress, (socklen_t *) &clientAddressLen);
        if (client_sock < 0) {
            return;
        }
        SimpleProtocolHeaders headers{};
        if (::recv(client_sock, &headers, sizeof(headers), 0) != sizeof(headers)) {
            return;
        }

        char *buffer = nullptr;
        if(headers.len > 0) {
            buffer = new char[headers.len];
            if (::recv(client_sock, buffer, headers.len, 0) != headers.len) {
                delete[] buffer;
                return;
            }
        }
        char *response = nullptr;
        size_t responseLen;
        if (_handlers.count(headers.type) == 0) {
            headers.type = 0;
            headers.isError = true;
            headers.len = strlen(NO_HANDLE_ERROR_MESSAGE);
            responseLen = headers.len;
            response = new char[headers.len];
            memcpy(response, NO_HANDLE_ERROR_MESSAGE, headers.len);
        } else {
            try {
                response = _handlers.at(headers.type)(_metadata, &clientAddress, buffer, headers.len, &responseLen);
                headers.type = 0;
                headers.isError = false;
                headers.len = responseLen;
            }
            catch (ProtocolException &exception) {
                response = new char[exception.whatSize()];
                memcpy(response, exception.what(), exception.whatSize());
                headers.type = 0;
                headers.isError = true;
                headers.len = exception.whatSize();
                responseLen = exception.whatSize();
            }
            delete[] buffer;
        }
        if (::send(client_sock, &headers, sizeof(headers), 0) != sizeof(headers)) {
            delete[] response;
            return;
        }
        if (::send(client_sock, response, responseLen, 0) != responseLen) {
            delete[] response;
            return;
        }
        delete[] response;
    }
};


#endif //CIOTA_COMPLETE_PROJECT_SIMPLE_TCP_PROTOCOL_SERVER_H
