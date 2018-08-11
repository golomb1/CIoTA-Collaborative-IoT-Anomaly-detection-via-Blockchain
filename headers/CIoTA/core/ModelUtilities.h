//
// Created by master on 10/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_MODEL_UTILITIES_H
#define CIOTA_COMPLETE_PROJECT_MODEL_UTILITIES_H

#include <atomic>
#include <condition_variable>
#include "../../CppUtils/MemBuffer.h"
#include "../../AnomalyDetection/AnomalyDetectionModel.h"

namespace CIoTA {

    /**
     * Define an interface for anomaly detection model utilities.
     * It is a bridge between the CIoTA agent and the model.
     * @tparam MODEL is the anomaly detection model class that this class mange.
     */

    template<typename MODEL>
    class ModelUtilities {
    public:

        /**
         * Combine a set of models into an combine model.
         * @param set of serialized models.
         * @return pointer to the combined model.
         * @attention the caller must delete the pointer after use.
         */
        virtual MODEL *combine(std::vector <MemBuffer*> const *set) = 0;

        /**
         * Test a given model and return his validation score.
         * @param model to test.
         * @return the validation score.
         */
        virtual size_t test(const MODEL *model) = 0;

        /**
         * Accept a new model, replace the correct model with the given model.
         * @param model to accept.
         */
        virtual void accept(MODEL *model) = 0;

        /**
         * Execute model.
         * @param cv is a condition variable that can be used to notify the model when to stop
         * (in case he run periodically).
         * @param shouldStop is a flag that indicate whether the model should stop executing.
         */
        virtual void execute(std::condition_variable *cv, std::atomic<bool> *shouldStop) = 0;

        /**
         * @out is a pointer to hold the serialization of the currect model.
         */
        virtual void getModel(MemBuffer* out) = 0;

        virtual bool isTesting() = 0;

        virtual void setListener(AnomalyListener *pListener) = 0;
    };
}

#endif //CIOTA_COMPLETE_PROJECT_MODEL_UTILITIES_H
