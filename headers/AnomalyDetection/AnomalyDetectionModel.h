//
// Created by golombt on 28/12/2017.
//

#ifndef CIOTA_COMPLETE_PROJECT_ANOMALY_DETECTION_MODEL_H
#define CIOTA_COMPLETE_PROJECT_ANOMALY_DETECTION_MODEL_H


#include <cstdint>
#include "../CppUtils/MemBuffer.h"


/**
 *  @file    AnomalyDetectionModel.h
 *  @author  Tomer Golomb (golombt)
 *  @version 1.0
 *
 *  @brief defines an interface for AnomalyListener class, and AnomalyDetectionModel class.
 *  AnomalyListener is an entity that can handle anomaly alerts.
 *  AnomalyDetectionModel is an entity that supports three operations:
 *  Comparison - > that return similarity of two models.
 *  consume -> give new observation to this model, the model can either learn from or alert on the observation.
 *  serialize -> turn the model into a buffer.
 *
 */


/**
 * @brief This class is an interface for listening on anomalies.
 */
class AnomalyListener{
public:
    /**
     * @brief Alert on a new anomaly.
     * @param score - the anomaly score.
     */
    virtual void anomalyAlert(uint64_t score) = 0;
};


/**
 * @brief Interface for anomaly detection model.
 * @tparam MODEL the type that implements this interface.
 * @tparam DATA_TYPE that this model accepts during training and testing.
 */
template <typename MODEL, typename DATA_TYPE>
class AnomalyDetectionModel{
public:
    /**
     * @brief Compare the base model against other model.
     * @param other model for comparison.
     * @return the difference percentage between the models.
     */
    virtual uint8_t compare(const MODEL* other) = 0;

    /**
     * @return serialize this model into a buffer.
     */
    virtual MemBuffer* serialize() = 0;

    /**
     * @brief Update the model with new data.
     * @param data
     */
    virtual void consume(DATA_TYPE data) = 0;
};


#endif //CIOTA_COMPLETE_PROJECT_ANOMALY_DETECTION_MODEL_H
