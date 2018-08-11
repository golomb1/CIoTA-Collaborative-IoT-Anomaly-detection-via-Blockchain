//
// Created by golombt on 28/12/2017.
//

#include <set>
#include <fcntl.h>
#include <unistd.h>
#include <headers/CppUtils/Logger.h>
#include "../../headers/JumpTracer/JumpTracer.h"
#include "../../headers/CppUtils/Cursor.h"



bool GLOBAL_ALERT_WAS_SEND = false;

JumpTracer::JumpTracer(AnomalyListener *alertListener,
                       size_t windows,
                       CodeAddress mask,
                       CodeAddress filterFrom, CodeAddress filterUntil,
                       uint64_t pThreshold) :
        _alertListener(alertListener),
        _sum(0),
        _event(windows),
        _trainFlag(true),
        _prev(0),
        _mask(mask),
        _filter(filterFrom, filterUntil),
        _pThreshold(pThreshold * windows),
        _alertWasSent(false)
{ }


JumpTracer::JumpTracer(const JumpTracer &other) : JumpTracer(&other)
{}


JumpTracer::JumpTracer(JumpTracer const *other) :
        _alertListener(other->_alertListener),
        _sum(other->_sum),
        _event(other->_event),
        _trainFlag(other->_trainFlag == true),
        _prev(other->_prev),
        _mask(other->_mask),
        _sums(other->_sums),
        _filter(other->_filter),
        _table(other->_table),
        _pThreshold(other->_pThreshold),
        _alertWasSent(false)
{}



CodeAddress JumpTracer::prepareAddress(CodeAddress address) {
    if (_filter.first <= address && address <= _filter.second) {
        return address & _mask;
    }
    else{
        return 0;
    }
}


uint64_t JumpTracer::grade(){
    if(!_trainFlag && _event.size() == _event.capacity()) {
        return _sum / _event.capacity();
    }
    else{
        return MAX_GRADE;
    }
}

void JumpTracer::deserialize(const char *buffer, size_t len) {
    Cursor c(buffer, len);
    size_t mapperSize;
    c.readInto(&mapperSize, sizeof(size_t));
    std::map<uint16_t , CodeAddress > mapper;
    for(size_t i=0; i < mapperSize; i++){
        CodeAddress address;
        c.readInto(&address, sizeof(address));
        mapper.insert(std::make_pair(i+1, address));
    }
    // read table
    while(c.hasMore()){
        uint16_t row;
        uint16_t rowLength;
        c.readInto(&row, sizeof(row));
        c.readInto(&rowLength, sizeof(rowLength));
        for(uint16_t i=0; i < rowLength; i++){
            uint16_t col;
            uint64_t value;
            c.readInto(&col, sizeof(col));
            c.readInto(&value, sizeof(value));
            addJump(mapper.at(row),mapper.at(col), value);
        }
    }
}


const std::map<CodeAddress, std::map<CodeAddress, uint64_t>> JumpTracer::table() const {
    return _table;
}

const std::map<CodeAddress, uint64_t> JumpTracer::sums() const {
    return _sums;
}

void JumpTracer::addJump(CodeAddress src, CodeAddress dst, uint64_t value) {
    this->_table[src][dst] += value;
    this->_sums[src] += value;
}

void JumpTracer::addJump(CodeAddress src, CodeAddress dst) {
    this->_table[src][dst] += 1;
    this->_sums[src] += 1;
}


size_t calcDistance(uint8_t first, uint8_t second){
    return static_cast<size_t>(abs(first - second));
}

uint8_t JumpTracer::compare(const JumpTracer * model) {
    size_t counter = 0;
    uint64_t distance = 0;
    auto myRowIt    = _table.begin();
    auto otherRowIt = model->_table.begin();
    while(myRowIt != _table.end() && otherRowIt != model->_table.end()){
        if(myRowIt->first == otherRowIt->first){
            // same source was observed both by this model and other model.
            auto myColIt    = myRowIt->second.begin();
            auto otherColIt = otherRowIt->second.begin();
            while(myColIt != myRowIt->second.end() && otherColIt != otherRowIt->second.end()){
                if(myColIt->first == otherColIt->first){
                    distance += calcDistance(calcProbability(myRowIt->first, myColIt->first),
                                             model->calcProbability(otherRowIt->first, otherColIt->first));
                    ++otherColIt;
                    ++myColIt;
                    counter++;
                }
                else if(myColIt->first > otherColIt->first){
                    // other have address that we doesn't have
                    ++otherColIt;
                    distance += MAX_GRADE;
                    counter++;
                }
                else{
                    // we have address that other doesn't have
                    ++myColIt;
                    //distance += MAX_GRADE;
                }
            }
            ++myRowIt;
            ++otherRowIt;
        }
        else if(myRowIt->first > otherRowIt->first){
            // other have address that we doesn't have
            distance += (MAX_GRADE * otherRowIt->second.size());
            counter+=otherRowIt->second.size();
            ++otherRowIt;
        }
        else{
            // we have address that other doesn't have
            ++myRowIt;
        }
    }
    for(;myRowIt != _table.end(); ++myRowIt){
        // nothing for now.
    }
    for(;otherRowIt != model->_table.end(); ++otherRowIt){
        distance += (MAX_GRADE * otherRowIt->second.size());
        counter+=otherRowIt->second.size();
    }
    if(counter == 0) {
        return 0;
    }
    else{
        return static_cast<uint8_t>(MAX_GRADE - (distance / counter));
    }
}

bool JumpTracer::serializeInto(MemBuffer *buffer) {
    size_t totalSize = sizeof(size_t);
    std::map<CodeAddress, uint16_t> mapper;
    {
        std::set<CodeAddress> addresses;
        for (auto row : _table) {
            totalSize += 2*sizeof(uint16_t);
            addresses.insert(row.first);
            for (auto col : row.second) {
                totalSize += sizeof(uint16_t) + sizeof(uint64_t);
                addresses.insert(col.first);
            }
        }
        totalSize += (sizeof(CodeAddress)*addresses.size());
        uint16_t i=0;
        for(CodeAddress it : addresses){
            mapper.insert(std::make_pair(it, i+1));
            i++;
        }
    }
    size_t mapperSize = mapper.size();
    buffer->reserveMore(totalSize);
    buffer->append(&mapperSize, sizeof(size_t));
    for(auto it : mapper){
        CodeAddress value = it.first;
        buffer->append(&value, sizeof(CodeAddress));
    }
    for(auto row : _table){
        uint16_t rowIndex = mapper.at(row.first);
        auto rowLen = static_cast<uint16_t>(row.second.size());
        buffer->append(&rowIndex, sizeof(uint16_t));
        buffer->append(&rowLen, sizeof(uint16_t));
        for(auto col : row.second){
            uint16_t colIndex  = mapper.at(col.first);
            buffer->append(&colIndex, sizeof(uint16_t));
            buffer->append(&(col.second), sizeof(uint64_t));
        }
    }
    LOG_TRACE_3("Propagate a model of size {0}", buffer->size());
    return true;
}

MemBuffer *JumpTracer::serialize() {
    auto* buffer = new MemBuffer();
    serializeInto(buffer);
    return buffer;
}

uint8_t JumpTracer::calcProbability(CodeAddress from, CodeAddress to) const {
    if(_table.count(from) > 0){
        if(_table.at(from).count(to) > 0){
            return static_cast<uint8_t>((_table.at(from).at(to) * MAX_GRADE) / (_sums.at(from)));
        }
    }
    return 0;
}

void JumpTracer::consume(CodeAddress data) {
    bool flag = false;
    CodeAddress dst = prepareAddress(data);
    if(_trainFlag.load()) {
        addJump(_prev, dst);
    }
    else {//if(!(_prev == 0 || dst == 0)){
        uint8_t prob = calcProbability(_prev, dst);
        _sum += prob;
        //std::cout << "Got " << _sum << "," << prob << "\n";
        //std::cerr <<_event.size() << "|"<<_event.capacity() <<"\n";
        if(_event.size() == _event.capacity()){
            //std::cerr <<  _sum << "," << (int)prob << "\n";
	    _sum -= _event.last();
            //LOG_TRACE_2("JUMP data is {0}", data);
	    if(_sum < _pThreshold || flag){
      //std::cerr <<  _sum << ",!!!!!!!!" << (int)prob << "\n";
      if(!_alertWasSent || !GLOBAL_ALERT_WAS_SEND){
		  int fd = open("/tmp/pibrellaFifo", O_WRONLY);
		  if(fd >= 0){
		      char buffer = 'r';
		      write(fd, &buffer, sizeof(char));
		      close(fd);
		  }
		  fd = open("/tmp/pibrellaFifo", O_WRONLY);
		  if(fd >= 0){
		      char buffer = 'a';
		      write(fd, &buffer, sizeof(char));
		      close(fd);
		  }
		  _alertWasSent = true;
		  GLOBAL_ALERT_WAS_SEND = true;
		}
	        if(_alertListener != nullptr){
                    _alertListener->anomalyAlert(_sum);
                }
            }
            else{
                //addJump(_event.last(), _event.fromLast(1));
            }
        }
        _event.push_back(prob);
    }
    _prev = dst;
}

void JumpTracer::setListener(AnomalyListener *alertListener) {
    _alertListener = alertListener;
}

bool JumpTracer::isTesting() {
    return !(_trainFlag.load());
}

