//
// Created by golombt on 17/12/2017.
//

#ifndef CPP_UTILS_EXCEPTIONS_H
#define CPP_UTILS_EXCEPTIONS_H

#include <system_error>
#include <cstring>
#include <string>
#include <cstdarg>
#include <cstdarg>
#include <utility>

/**
 *  @file    Exception.h
 *  @author  Tomer Golomb (golombt)
 *  @version 1.0
 *
 *  @brief defines different kind of exceptions.
 *
 *  Defines the following exceptions:
 *      MessageException.
 *      IndexOutOfBoundException.
 */

class MessageException: public std::exception{
public:
    /** Constructor (C strings).
     *  @param message C-style string error message.
     *                 The string contents are copied upon construction.
     *                 Hence, responsibility for deleting the char* lies
     *                 with the caller.
     */
    explicit MessageException(char const* fmt, ...) __attribute__((format(printf,2,3))){
        va_list ap;
        va_start(ap, fmt);
        char text[1024];
        vsnprintf(text, sizeof text, fmt, ap);
        _msg.assign(text, strlen(text));
        va_end(ap);
    }

    /** Constructor (C++ STL strings).
     *  @param message The error message.
     */
    explicit MessageException(std::string message):
            _msg(std::move(message)){ }


    MessageException(const MessageException& other) noexcept :
        _msg(other._msg){ }



    /** Destructor.
     * Virtual to allow for subclassing.
     */
    ~MessageException() throw () override = default;

    /** Returns a pointer to the (constant) error description.
     *  @return A pointer to a const char*. The underlying memory
     *          is in position of the Exception object. Callers must
     *          not attempt to free the memory.
     */
    const char* what() const throw () override {
        return _msg.c_str();
    }

    size_t whatSize() const {
        return _msg.size();
    }

protected:
    /** Error message. */
    std::string _msg;
};


class IndexOutOfBoundException : public MessageException{
public:
    explicit IndexOutOfBoundException(const std::string& msg, size_t got, size_t max) :
            MessageException("%s - IndexOutOfBound: got: %zu but max is %zu.", msg.c_str(), got, max){}
    explicit IndexOutOfBoundException(const std::string& message): MessageException(message){ }
};

class InvalidStateException : public MessageException {
public:
    explicit InvalidStateException(const std::string &msg, const std::string &stateError) :
            MessageException("%s - InvalidStateException: %s.", msg.c_str(), stateError.c_str()) {}

    explicit InvalidStateException(const std::string &message) : MessageException(message) {}
};


class SystemError : public std::system_error{
public:
    explicit SystemError(const std::string& errMsg) : std::system_error(errno, std::system_category(), errMsg.c_str()){}
};
#endif //CPP_UTILS_EXCEPTIONS_H
