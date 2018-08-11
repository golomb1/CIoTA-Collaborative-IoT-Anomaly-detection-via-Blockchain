//
// Created by master on 18/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_VIEWER_UTILITIES_H
#define CIOTA_COMPLETE_PROJECT_VIEWER_UTILITIES_H

#include "core/ModelUtilities.h"
#include "../CppUtils/Exceptions.h"
#include "../Blockchain/BlockchainListener.h"
#include "core/CIoTABlockchainApplication.h"

namespace CIoTA {

    template<typename MODEL>
    class ObserverListener {

    public:
        virtual void reportCombining(std::vector<MemBuffer *> const *pVector, MODEL *pMODEL) = 0;

        virtual void reportCombining() = 0;

        virtual void reportTestResult(size_t grade)= 0;

        virtual void reportTestResult()= 0;

        virtual void reportAcceptance()= 0;
    };

    /**
     * Observer utilities that can be used by observer instead of a ModelUtilities.
     * @tparam MODEL is the type of anomaly detection model used by the CIoTA network.
     */
    template<typename MODEL>
    class ObserverUtilities : public ModelUtilities<MODEL> {
    private:
        ModelUtilities <MODEL> *_realUtilities;
        ObserverListener<MODEL> *_listener;
    public:

        void setListener(AnomalyListener *pListener) override {
            // nothing.
        }

        /**
         * Constructor
         * @param realUtilities an implementation of ModelUtilities to delegate this object function, might be nullptr.
         * @param listener to events on the network.
         */
        explicit ObserverUtilities(ModelUtilities <MODEL> *realUtilities, ObserverListener<MODEL> *listener) :
                _realUtilities(realUtilities), _listener(listener) {}

        MODEL *combine(std::vector<MemBuffer *> const *set) override {
            if (_realUtilities != nullptr) {
                MODEL *res = _realUtilities->combine(set);
                if (_listener != nullptr) {
                    _listener->reportCombining(set, res);
                }
                return res;
            } else {
                if (_listener != nullptr) {
                    _listener->reportCombining();
                }
                return nullptr;
            }
        }

        size_t test(const MODEL *model) override {
            // always pass
            if (_realUtilities != nullptr) {
                size_t res = _realUtilities->test(model);
                if (_listener != nullptr) {
                    _listener->reportTestResult(res);
                }
                return res;
            } else {
                if (_listener != nullptr) {
                    _listener->reportTestResult();
                }
                return 100;
            }
        }

        void accept(MODEL *model) override {
            if (_realUtilities != nullptr) {
                _realUtilities->accept(model);
            }
            if (_listener != nullptr) {
                _listener->reportAcceptance();
            }
        }

        void execute(std::condition_variable *cv, std::atomic<bool> *shouldStop) override {
            throw InvalidStateException("ObserverUtilities::executeModel should not be called");
        }

        void getModel(MemBuffer *out) override {
            throw InvalidStateException("ObserverUtilities::getModel should not be called");
        }

        bool isTesting() override {
            throw InvalidStateException("ObserverUtilities::isTesting should not be called");
        }

    };

}
#endif //CIOTA_COMPLETE_PROJECT_VIEWER_UTILITIES_H
