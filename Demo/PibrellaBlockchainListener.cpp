//
// Created by master on 08/02/18.
//

#include <sys/types.h>
#include <fcntl.h>
#include <zconf.h>
#include "PibrellaBlockchainListener.h"

void PibrellaBlockchainListener::onBlockAcceptance(const char *, size_t ) {
    int fd = open("/tmp/pibrellaFifo", O_WRONLY);
    if(fd >= 0){
        char buffer = 's';
        write(fd, &buffer, sizeof(char));
        close(fd);
    }
}

void PibrellaBlockchainListener::onPartialBlockAcceptance(const char *, size_t ) {

}

void PibrellaBlockchainListener::onPartialBlockAcceptance(const char *, size_t , bool selfUpdate) {
    int fd = open("/tmp/pibrellaFifo", O_WRONLY);
    if(fd >= 0){
        char buffer = 'b';
        write(fd, &buffer, sizeof(char));
        close(fd);
    }
    /*if(_fd >= 0 && !selfUpdate) {
        char buffer = 'z';
        write(_fd, &buffer, sizeof(char));
        buffer = 'b';
        write(_fd, &buffer, sizeof(char));
    }*/
}

void PibrellaBlockchainListener::finishTraining() {
    int fd = open("/tmp/pibrellaFifo", O_WRONLY);
    if(fd >= 0){
        char buffer = 'g';
        write(fd, &buffer, sizeof(char));
        close(fd);
    }
    fd = open("/tmp/pibrellaFifo", O_WRONLY);
    if(fd >= 0){
        char buffer = 'y';
        write(fd, &buffer, sizeof(char));
        close(fd);
    }
    /*if(_fd >= 0) {
        char buffer = 'y';
        write(_fd, &buffer, sizeof(char));
        buffer = 'g';
        write(_fd, &buffer, sizeof(char));
    }*/
}

PibrellaBlockchainListener::PibrellaBlockchainListener() : _fd(-1) {
    int fd = open("/tmp/pibrellaFifo", O_WRONLY);
    if(fd >= 0){
        char buffer = 'y';
        write(fd, &buffer, sizeof(char));
        close(fd);
    }
    /*_fd = open("/tmp/pibrellaFifo", O_WRONLY);
    if(_fd >= 0) {
        char buffer = 'y';
        write(_fd, &buffer, sizeof(char));
    }*/
}

PibrellaBlockchainListener::~PibrellaBlockchainListener() {
    /*if(_fd >= 0) {
        close(_fd);
    }*/
}

void PibrellaBlockchainListener::setCompleteBlockSize(size_t) {}


