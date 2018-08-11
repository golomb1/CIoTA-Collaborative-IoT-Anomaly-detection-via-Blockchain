//
// Created by master on 10/01/18.
//

#include "../../headers/CppUtils/Logger.h"

#if LOG_LEVEL > 0
#if LOG_TO_CONSOLE
std::shared_ptr<spdlog::logger> logger = spdlog::stdout_logger_mt("console");
#else
std::shared_ptr<spdlog::logger> logger = spdlog::daily_logger_mt("FileLogger", "/home/pi/Logs/CIoTA_FileLog.txt");
#endif
#endif
