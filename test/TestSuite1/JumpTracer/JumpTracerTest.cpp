//
// Created by golombt on 28/12/2017.
//

#include "JumpTracerTest.h"
#include "../../../headers/JumpTracer/JumpTracer.h"
#include "../../TEST.h"


bool anomalyTest(){
    std::cout << "\tanomalyTest\n";
    size_t windows = 1000;
    uint64_t start = 0x00010000;
    uint64_t end = 0x0001a000;
    uint64_t range = end-start;
    uint32_t threshold = 50;
    JumpTracer tracer(windows, static_cast<uint64_t>((~0) ^ 0x00000011),
                      start, end, threshold);

    uint64_t index = 1;
    for(uint64_t i=0; i < 100000; i++){
        uint64_t pc = start + index;
        tracer.consume(pc);
        index *= 15;
        if(index > range){
            index = 1;
        }
    }
    tracer.startTesting();
    index = 1;
    for(uint64_t i=0; i < 100000; i++){
        uint64_t pc = start + index;
        tracer.consume(pc);
        if(i > windows) {
            ASSERT(tracer.grade(), 100)
        }
        index *= 15;
        if(index > range){
            index = 1;
        }
    }
    return true;
}


bool serializeTest(){
    auto mask = static_cast<uint64_t>((~0) ^ 0x00000011);
    std::cout << "\tserializeTest\n";
    size_t windows = 1000;
    uint64_t start = 0x00010000;
    uint64_t end = 0x0001a000;
    uint64_t range = end-start;
    uint32_t threshold = 50;
    MemBuffer *s = nullptr;
    {
        JumpTracer tracer(windows, mask,
                          start, end, threshold);

        uint64_t index = 1;
        for (uint64_t i = 0; i < 100000; i++) {
            uint64_t pc = start + index;
            tracer.consume(pc);
            index *= 15;
            if (index > range) {
                index = 1;
            }
        }
        s = tracer.serialize();
    }
    JumpTracer tracer2(windows, mask, start, end, threshold);
    tracer2.deserialize(s->data(), s->size());
    delete s;
    tracer2.startTesting();
    uint64_t index = 1;
    for(uint64_t i=0; i < 100000; i++){
        uint64_t pc = start + index;
        tracer2.consume(pc);
        if(i > windows) {
            ASSERT(tracer2.grade(), 100)
        }
        index *= 15;
        if(index > range){
            index = 1;
        }
    }
    return true;
}

bool compareTest() {
    std::cout << "\tcompareTest\n";
    auto mask = static_cast<uint64_t>((~0) ^ 0x000000FF);
    size_t windows = 1000;
    uint64_t start = 0x00010000;
    uint64_t end = 0x0001a000;
    uint64_t range = end-start;
    uint32_t threshold = 50;

    JumpTracer tracer(windows, mask,
                      start, end, threshold);

    uint64_t index = 1;
    for (uint64_t i = 0; i < 100000; i++) {
        uint64_t pc = start + index;
        tracer.consume(pc);
        index *= 15;
        if (index > range) {
            index = 1;
        }
    }
    MemBuffer* s = tracer.serialize();
    JumpTracer tracer2(windows, mask, start, end, threshold);
    tracer2.deserialize(s->data(), s->size());
    ASSERT(tracer.compare(&tracer2), 100);
    delete s;
    {
        JumpTracer tracer3(windows, mask,
                           start, end, threshold);
        index = 1;
        for (uint64_t i = 0; i < 100000; i++) {
            uint64_t pc = start + index + 15;
            tracer3.consume(pc);
            index *= 15;
            if (index > range) {
                index = 1;
            }
        }
        ASSERT(tracer.compare(&tracer3), 100);
    }

    JumpTracer tracer4(windows, mask,
                       start, end, threshold);

    index = 256;
    for (uint64_t i = 0; i < 100000; i++) {
        uint64_t pc = start + index;
        tracer4.consume(pc);
        index *= 15;
        if (index > range) {
            index = 256;
        }
    }
    ASSERT(tracer.compare(&tracer4), 0);
    return true;
}



bool anomalyAttackTest(){
    std::cout << "\tanomalyAttackTest\n";
    size_t windows = 1000;
    uint64_t start = 0x00010000;
    uint64_t end = 0x0001a000;
    uint64_t range = end-start;
    uint32_t threshold = 50;
    JumpTracer tracer(windows, static_cast<uint64_t>((~0) ^ 0x00000011),
                      start, end, threshold);

    uint64_t index = 1;
    for(uint64_t i=0; i < 100000; i++){
        uint64_t pc = start + index;
        tracer.consume(pc);
        index *= 15;
        if(index > range){
            index = 1;
        }
    }
    tracer.startTesting();
    index = 17;
    for(uint64_t i=0; i < 100000; i++){
        uint64_t pc = start + index;
        tracer.consume(pc);
        if(i > windows) {
            ASSERT(tracer.grade(), 0)
        }
        index *= 15;
        if(index > range){
            index = 17;
        }
    }
    return true;
}

bool JumpTableTest(){
    std::cout << "JumpTableTest" << std::endl;
    return anomalyTest() && serializeTest() && compareTest() && anomalyAttackTest();
}