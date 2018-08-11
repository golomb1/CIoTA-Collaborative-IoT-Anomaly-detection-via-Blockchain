//
// Created by master on 10/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_LOGGER_H
#define CIOTA_COMPLETE_PROJECT_LOGGER_H

#include <memory>

#if LOG_LEVEL > 0

#include <spdlog/spdlog.h>


extern std::shared_ptr<spdlog::logger> logger;


template <typename... Args>
void LogInfo(const char* fmt, const Args&... args){
    logger->info(fmt, args...);
}

#define LOG_TRACE(...)       LOG_TRACE_1(__VA_ARGS__);
#else

#define LOG_TRACE(...)

#endif


#if LOG_LEVEL >= 1
#define LOG_TRACE_1(...)     LogInfo(__VA_ARGS__);
#endif
#if LOG_LEVEL >= 2
#define LOG_TRACE_2(...)     LogInfo(__VA_ARGS__);
#endif
#if LOG_LEVEL >= 3
#define LOG_TRACE_3(...)     LogInfo(__VA_ARGS__);
#endif
#if LOG_LEVEL >= 4
#define LOG_TRACE_4(...)     LogInfo(__VA_ARGS__);
#endif
#if LOG_LEVEL >= 5
#define LOG_TRACE_5(...)     LogInfo(__VA_ARGS__);
#endif

#if LOG_LEVEL < 1
#define LOG_TRACE_1(...)
#endif
#if LOG_LEVEL < 2
#define LOG_TRACE_2(...)
#endif
#if LOG_LEVEL < 3
#define LOG_TRACE_3(...)
#endif
#if LOG_LEVEL < 4
#define LOG_TRACE_4(...)
#endif
#if LOG_LEVEL < 5
#define LOG_TRACE_5(...)
#endif


#endif //CIOTA_COMPLETE_PROJECT_LOGGER_H
