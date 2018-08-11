//
// Created by golombt on 28/12/2017.
//

#include <utility>

#include "../../headers/JumpTracer/JumpTracerUtilities.h"




JumpTracerUtilities::JumpTracerUtilities(pid_t _pid,
                                         size_t _pConsensus,
                                         size_t _consensusSize,
                                         size_t trainInterval,
                                         JumpTracer *_empty,
                                         bool(*source)(LockedPointer<JumpTracer> *, std::atomic<bool> *, pid_t)) :
        _pid(_pid),
        _pConsensus(_pConsensus),
        _consensusSize(_consensusSize),
        _trainInterval(trainInterval),
        _empty(_empty),
        _model(new JumpTracer(_empty)),
        _source(source),
        _callback(nullptr),
        _alertListener(nullptr)
{}


JumpTracerUtilities::~JumpTracerUtilities(){
    JumpTracer *old = _model.update(nullptr);
    delete old;

}

bool JumpTracerUtilities::isTesting() {
    bool res = false;
    this->_model.use(&JumpTracer::isTesting,&res);
    return res;
}

JumpTracer *JumpTracerUtilities::combine(const std::vector<MemBuffer*>* set) {
    std::map<uint64_t, std::map<uint64_t, uint64_t>> sums;
    std::map<uint64_t, std::map<uint64_t, uint64_t>> count;
    for(MemBuffer* buffer : *set){
        auto tracer = createEmptyModel();
        tracer->deserialize(buffer->data(), buffer->size());
        for(auto row : tracer->table()){
            for(auto column : row.second){
                sums[row.first][column.first] += column.second;
                count[row.first][column.first]++;
            }
        }
        delete tracer;
    }
    auto* combined = createEmptyModel();
    for(auto row :  sums){
        for(auto column : row.second){
            if(count[row.first][column.first] >= _pConsensus || set->size() < _consensusSize){
                combined->addJump(row.first, column.first,sums[row.first][column.first]);
            }
        }
    }
    combined->startTesting();
    return combined;
}

size_t JumpTracerUtilities::test(const JumpTracer *model) {
    uint8_t score = 0;
    if(_model.use(&JumpTracer::compare, &score, model)){
       return score;
    }
    else{
        return 0;
    }
}

void JumpTracerUtilities::accept(JumpTracer *model) {
    if(model != nullptr){
	model->setListener(_alertListener);
    }
    JumpTracer* old = _model.update(model);
    if(old != nullptr) {
        delete (old);
    }
}


void JumpTracerUtilities::execute(std::condition_variable* cv, std::atomic<bool> *shouldStop) {
    LOG_TRACE_2("JumpTracerUtilities train for {0}", _trainInterval);
    std::thread trainWaiter(workerDelayedThread<JumpTracerUtilities*>, this,
                            _trainInterval, cv, shouldStop, &JumpTracerUtilities::startTesting);
    _source(&_model, shouldStop, _pid);
    trainWaiter.join();
}

void JumpTracerUtilities::startTesting(JumpTracerUtilities* utilities){
    LOG_TRACE_2("JumpTracerUtilities - start testing")
    if(utilities->_callback != nullptr){
	utilities->_callback();
    }
    utilities->_model.use(&JumpTracer::startTesting);
}

void JumpTracerUtilities::getModel(MemBuffer * buffer) {
    bool res;
    _model.use(&JumpTracer::serializeInto, &res, buffer);
    if(!res){
        buffer->clear();
    }
}

JumpTracer *JumpTracerUtilities::createEmptyModel() const {
    return new JumpTracer(_empty);
}

const std::map<CodeAddress, std::map<CodeAddress, uint64_t>> JumpTracerUtilities::getTable() {
    return _model.get()->table();
}

void JumpTracerUtilities::setListener(AnomalyListener *pListener) {
    _model.use(&JumpTracer::setListener, pListener);
    _alertListener = pListener;
}

