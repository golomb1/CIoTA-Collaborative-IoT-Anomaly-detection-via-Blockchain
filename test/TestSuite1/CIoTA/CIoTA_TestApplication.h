//
// Created by golombt on 27/12/2017.
//

#ifndef CIOTA_COMPLETE_PROJECT_CIOTA_TEST_H
#define CIOTA_COMPLETE_PROJECT_CIOTA_TEST_H

#include "../../../headers/CIoTA/core/Agent.h"
#include "../../TEST.h"


class StringModelUtilities : public CIoTA::ModelUtilities<std::string>{
public:
    std::string *combine(const std::vector<MemBuffer*>* set) override {
        for(MemBuffer * m : *set){
            ASSERT(memcmp(m->data(), "Model", strlen("Model")),0)
        }
        return new std::string("Combine Content");
    }


    size_t test(const  std::string *model) override {
        ASSERT(model->size(), strlen("Combine Content"));
        ASSERT(memcmp(model->data(), "Combine Content", strlen("Combine Content")), 0);
        return 100;
    }

    void accept(std::string *model) override {
        delete model;
    }

    void execute(std::condition_variable* cv, std::atomic<bool> *shouldStop) override {

    }

    void getModel(MemBuffer* model) override {
        model->append("Model", strlen("Model"));
    }

    bool isTesting() override {
        return true;
    }

    void setListener(AnomalyListener *pListener) override {
    }
};


class EmptySHA256CryptoLibraryTest : public CryptoLibrary<SHA256_OUTPUT_SIZE> {
private:
    std::map<int , MemBuffer> mapper;
    int index = 0;
    SHA256_FUNC _sha;
public:
    EmptySHA256CryptoLibraryTest() : CryptoLibrary(&_sha, this) {};

    bool unseal(const char* from, size_t fromLen, const char* content, size_t contentLen, MemBuffer* out) override{
        int i = *(int*)content;
        MemBuffer m = mapper.at(i);
        out->append(m.data(),m.size());
        return true;
    }

    bool seal(const char *from, size_t fromLen, const char *content, size_t contentLen, MemBuffer *out) override {
        mapper.insert(std::make_pair(index, MemBuffer(content, contentLen)));
        out->append(&index, sizeof(int));
        index++;
        return true;
    }
};



bool CIoTA_TestMain();

#endif //CIOTA_COMPLETE_PROJECT_CIOTA_TEST_H
