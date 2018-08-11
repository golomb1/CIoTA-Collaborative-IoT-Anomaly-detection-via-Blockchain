//
// Created by golombt on 28/12/2017.
//

#ifndef CIOTA_COMPLETE_PROJECT_JUMP_TRACER_UTILITIES_H
#define CIOTA_COMPLETE_PROJECT_JUMP_TRACER_UTILITIES_H

#include "JumpTracer.h"
#include "../CIoTA/core/Agent.h"
#include "../CppUtils/LockedPointer.h"

class JumpTracerUtilities : public CIoTA::ModelUtilities<JumpTracer> {
private:
    pid_t _pid;
    size_t _pConsensus;
    size_t _consensusSize;
    size_t _trainInterval;
    JumpTracer* _empty;
    LockedPointer<JumpTracer> _model;
    bool(*_source)(LockedPointer<JumpTracer>*, std::atomic<bool>*, pid_t);
    std::function<void(void)> _callback;
    AnomalyListener *_alertListener{};


public:
    JumpTracerUtilities(pid_t _pid, size_t _pConsensus, size_t _consensusSize, size_t trainInterval, JumpTracer *_empty,
                        bool(*source)(LockedPointer<JumpTracer> *, std::atomic<bool> *, pid_t));

    ~JumpTracerUtilities();

    JumpTracer* combine(const std::vector<MemBuffer*>* set) override;

    size_t test(const  JumpTracer *model) override;

    void accept(JumpTracer *model) override;

    void execute(std::condition_variable* cv, std::atomic<bool> *shouldStop) override;

    void getModel(MemBuffer * buffer) override;

    JumpTracer *createEmptyModel() const;

    static void startTesting(JumpTracerUtilities* utilities);

    bool isTesting() override;

    const std::map<CodeAddress, std::map<CodeAddress, uint64_t>> getTable();

    void setListener(AnomalyListener *pListener) override;
    
    void setTrainingCallback(std::function<void(void)> lambda) { _callback = lambda;}
};


#endif //CIOTA_COMPLETE_PROJECT_JUMP_TRACER_UTILITIES_H
