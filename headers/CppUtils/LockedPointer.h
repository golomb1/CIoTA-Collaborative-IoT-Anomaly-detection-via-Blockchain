//
// Created by golombt on 17/12/2017.
//

#ifndef CPP_UTILS_LOCKED_POINTER_H
#define CPP_UTILS_LOCKED_POINTER_H

#include <mutex>

/**
 *  @file    LockedPointer.h
 *  @author  Tomer Golomb (golombt)
 *  @version 1.0
 *
 *  @brief defines an interface for LockedPointer class.
 *  LockedPointer is used to ease the handling of multi-threading pointer management.
 */
template <typename T>
class LockedPointer{
public:
    // Constructor & Destructor
    explicit LockedPointer(T* ptr){
        _ptr = ptr;
    }
    virtual ~LockedPointer(){
    }

    T* update(T* ptr){
        T* tmp = nullptr;
        _mtx.lock();
        tmp = _ptr;
        _ptr = ptr;
        _mtx.unlock();
        return tmp;
    }

    template<typename R, typename P1, typename P2, typename P3>
    bool use(R (T::*func)(P1 p1, P2 p2, P3 p3), R* out, P1 p1, P2 p2, P3 p3){
        bool ret = true;
        _mtx.lock();
        if(_ptr == NULL){
            ret = false;
        }
        else{
            if(out == NULL){
                (_ptr->*func)(p1, p2, p3);
            }
            else{
                *out = (_ptr->*func)(p1, p2, p3);
            }
        }
        _mtx.unlock();
        return ret;
    }


    template<typename R, typename P1, typename P2, typename P3, typename P4>
    bool use(R (T::*func)(P1 p1, P2 p2, P3 p3, P4 p4), R* out, P1 p1, P2 p2, P3 p3, P4 p4){
        bool ret = true;
        _mtx.lock();
        if(_ptr == NULL){
            ret = false;
        }
        else{
            if(out == NULL){
                (_ptr->*func)(p1, p2, p3, p4);
            }
            else{
                *out = (_ptr->*func)(p1, p2, p3, p4);
            }
        }
        _mtx.unlock();
        return ret;
    }

    template<typename R, typename P1, typename P2>
    bool use(R (T::*func)(P1 p1, P2 p2), R* out, P1 p1, P2 p2){
        bool ret = true;
        _mtx.lock();
        if(_ptr == NULL){
            ret = false;
        }
        else{
            if(out == NULL){
                (_ptr->*func)(p1, p2);
            }
            else{
                *out = (_ptr->*func)(p1, p2);
            }
        }
        _mtx.unlock();
        return ret;
    }

    template<typename P1, typename P2>
    bool use(void (T::*func)(P1 p1, P2 p2), P1 p1, P2 p2){
        bool ret = true;
        _mtx.lock();
        if(_ptr == NULL){
            ret = false;
        }
        else{
            (_ptr->*func)(p1, p2);
        }
        _mtx.unlock();
        return ret;
    }

    template<typename R, typename P1>
    bool use(R (T::*func)(P1 p1), R* out, P1 p1){
        return useAndReturn(func, out, p1);
    };

    template<typename R, typename P1>
    bool useAndReturn(R (T::*func)(P1 p1), R* out, P1 p1){
        bool ret = true;
        _mtx.lock();
        if(_ptr == NULL){
            ret = false;
        }
        else{
            if(out == NULL){
                (_ptr->*func)(p1);
            }
            else{
                *out = (_ptr->*func)(p1);
            }
        }
        _mtx.unlock();
        return ret;
    }


    template<typename P1>
    bool use(void (T::*func)(P1 p1), P1 p1){
        bool ret = true;
        _mtx.lock();
        if(_ptr == NULL){
            ret = false;
        }
        else{
            (_ptr->*func)(p1);
        }
        _mtx.unlock();
        return ret;
    }


    template<typename R>
    bool use(R (T::*func)(), R* out){
        bool ret = true;
        _mtx.lock();
        if(_ptr == NULL){
            ret = false;
        }
        else{
            if(out == NULL){
                (_ptr->*func)();
            }
            else{
                *out = (_ptr->*func)();
            }
        }
        _mtx.unlock();
        return ret;
    }

    bool use(void (T::*func)()){
        bool ret = true;
        _mtx.lock();
        if(_ptr == NULL){
            ret = false;
        }
        else{
            (_ptr->*func)();
        }
        _mtx.unlock();
        return ret;
    }


    T* get(){
        T* ret = NULL;
        _mtx.lock();
        ret = _ptr;
        _mtx.unlock();
        return ret;
    }


private:
    T*         _ptr;
    std::mutex _mtx;
};

#endif //CPP_UTILS_LOCKED_POINTER_H
