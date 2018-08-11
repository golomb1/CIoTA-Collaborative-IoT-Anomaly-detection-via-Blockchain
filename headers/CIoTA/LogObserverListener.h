//
// Created by tomer on 21/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_LOG_OBSERVER_LISTENER_H
#define CIOTA_COMPLETE_PROJECT_LOG_OBSERVER_LISTENER_H

#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include "ObserverUtilities.h"

template<typename MODEL>
class LogObserverListener : public CIoTA::ObserverListener<MODEL>{
private:
    std::shared_ptr<spdlog::logger> logger;

public:

    LogObserverListener() {
        logger = spdlog::stdout_logger_mt("LogObserverListener");
    }


    LogObserverListener(char *string, size_t loggerSize, size_t rotation) {
        logger = spdlog::rotating_logger_mt("observerLogger", "logs/observer.log", loggerSize, rotation);
    }


    void reportCombining(std::vector<MemBuffer *> const *pVector, MODEL *pMODEL) override {
        logger->info("Combined set of {0:d} records", pVector->size());
    }

    void reportCombining() override {
        logger->info("Combined set of records");
    }

    void reportTestResult(size_t grade) override {
        logger->info("Combination grade is {0:d}", grade);
    }

    void reportTestResult() override {
        logger->info("Test performed");
    }

    void reportAcceptance() override {
        logger->info("Accepted new block");
    }

};
#endif //CIOTA_COMPLETE_PROJECT_LOG_OBSERVER_LISTENER_H
