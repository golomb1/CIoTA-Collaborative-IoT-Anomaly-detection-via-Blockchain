//
// Created by golombt on 28/12/2017.
//

#ifndef CIOTA_COMPLETE_PROJECT_JUMP_TRACER_H
#define CIOTA_COMPLETE_PROJECT_JUMP_TRACER_H


#include <cstdint>
#include <atomic>
#include <map>
#include "../CppUtils/CircularBuffer.h"
#include "../CppUtils/MemBuffer.h"
#include "../AnomalyDetection/AnomalyDetectionModel.h"


#define MAX_GRADE 100

extern bool GLOBAL_ALERT_WAS_SEND;


using CodeAddress = uint64_t;

class JumpTracer : public AnomalyDetectionModel<JumpTracer, CodeAddress> {
private:
    AnomalyListener *_alertListener{};
    // the sum of _event
    uint64_t _sum{};
    // sliding windows on the jumps during tests.
    CircularBuffer<uint8_t> _event;
    // flag that indicate whether the model should train on or test the data of the application.
    std::atomic<bool> _trainFlag{};
    // previous address that was seen for jumping
    CodeAddress _prev{};
    // mask to reduce size and define address range.
    CodeAddress _mask{};
    // sums of rows
    std::map<CodeAddress, uint64_t> _sums{};
    // filter address, records all addresses in this range, the rest are consider zero.
    std::pair<CodeAddress, CodeAddress> _filter;
    // the table that count the jumps.
    std::map<CodeAddress, std::map<CodeAddress, uint64_t>> _table{};
    // threshold for detection.
    uint64_t _pThreshold{};
    bool _alertWasSent = false;

    /**
     * prepare address by masking it and filter it.
     * @param address to prepare
     * @return the prepared address.
     */
    CodeAddress prepareAddress(CodeAddress address);


public:

    /**
     * Constructor
     * @param alertListener used to notify on alerts.
     * @param windows is the size of the test windows.
     * @param mask is used to reduce size and define address range.
     * @param filterFrom address, will records all addresses from this offset.
     * @param filterUntil address, will records all addresses until this offset.
     * @param pThreshold for anomaly.
     */
    JumpTracer(AnomalyListener *alertListener,
               size_t windows,
               CodeAddress mask,
               CodeAddress filterFrom, CodeAddress filterUntil,
               uint64_t pThreshold);

    virtual ~JumpTracer() = default;

    /**
     * Constructor
     * @param windows is the size of the test windows.
     * @param mask is used to reduce size and define address range.
     * @param filterFrom address, will records all addresses from this offset.
     * @param filterUntil address, will records all addresses until this offset.
     * @param pThreshold for anomaly.
     */
    JumpTracer(size_t windows,
               CodeAddress mask,
               CodeAddress filterFrom, CodeAddress filterUntil,
               uint64_t pThreshold) :
            JumpTracer(nullptr, windows, mask,
                       filterFrom, filterUntil, pThreshold)
    {}


    /**
     * Copy constructor
     * @param other instance to copy
     */
    JumpTracer(const JumpTracer &other);

    /**
     * Copy constructor
     * @param other instance to copy
     */
    explicit JumpTracer(JumpTracer const *other);

    /**
     * Deserialize buffer table.
     * @param buffer to deserialize.
     * @param len of buffer.
     */
    void deserialize(const char *buffer, size_t len);

    /**
     * @return this object jump table.
     */
    const std::map<CodeAddress, std::map<CodeAddress, uint64_t>> table() const;

    /**
     * @return the sums of all the rows.
     */
    const std::map<CodeAddress, uint64_t> sums() const;

    /**
     * Add new jump to the model.
     * @param src of the jump.
     * @param dst of the jump.
     * @param value how many occurrence of that jump happens.
     */
    void addJump(CodeAddress src, CodeAddress dst, uint64_t value);

    /**
     * @return the current grade.
     */
    uint64_t grade();

    /**
     * Add new jump to the model.
     * @param src of the jump.
     * @param dst of the jump.
     */
    void addJump(CodeAddress src, CodeAddress dst);


    uint8_t calcProbability(CodeAddress from, CodeAddress to) const;


    uint8_t compare(const JumpTracer *model) override;

    /**
     * Serialize this model into a buffer.
     * @return buffer representation of this model.
     * @attention the user need to delete this pointer.
     */
    MemBuffer *serialize() override;

    /**
     * Serialize this model into a given buffer.
     * @param buffer is the output buffer of the serialization.
     * @return true if success and false otherwise.
     */
    bool serializeInto(MemBuffer *buffer);

    /**
     * Consume new data, either test it or train on it.
     * @param data represent a new observation.
     */
    void consume(CodeAddress data) override;

    /**
     * Change the model state to test mode, (stopping the training).
     */
    void startTesting() {
        _trainFlag = false;
    }

    void setListener(AnomalyListener *alertListener);

    bool isTesting();
};


#endif //CIOTA_COMPLETE_PROJECT_JUMP_TRACER_H
