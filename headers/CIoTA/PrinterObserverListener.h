//
// Created by master on 24/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_PRINTER_OBSERVER_LISTENER_H
#define CIOTA_COMPLETE_PROJECT_PRINTER_OBSERVER_LISTENER_H


#include <headers/CppUtils/MemBuffer.h>
#include "ObserverUtilities.h"

class Printer{
public:

    virtual void LogInfo(const char* fmt, ...)=0;
};

template<typename MODEL>
class PrinterObserverListener : public CIoTA::ObserverListener<MODEL>{
public:
    Printer* _printer;

    explicit PrinterObserverListener(Printer* printer)
            :_printer(printer)
    {}

    void reportCombining(std::vector<MemBuffer *> const *pVector, MODEL *pMODEL) override {
        _printer->LogInfo("Combined set of {0:d} records", pVector->size());
    }

    void reportCombining() override {
        _printer->LogInfo("Combined set of records");
    }

    void reportTestResult(size_t grade) override {
        _printer->LogInfo("Combination grade is {0:d}", grade);
    }

    void reportTestResult() override {
        _printer->LogInfo("Test performed");
    }

    void reportAcceptance() override {
        _printer->LogInfo("Accepted new block");
    }

};


#endif //CIOTA_COMPLETE_PROJECT_PRINTER_OBSERVER_LISTENER_H
