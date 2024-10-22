#pragma once

#include <cstdint>
#include <cassert>
#include <fmt/core.h>
#include <fmt/chrono.h>
#include <fmt/xchar.h>
#include "trace++/Logger.h"

namespace tracejj
{
template <typename... Args>
inline void LogPrintAndWrite(LogLevel level, uint32_t uMod, const char* fmt, Args&&... args)
{
    auto& logger = Logger::Instance();
    try
    {
        if (logger.MaxLevel() >= level)
        {
            const std::string strMessage(fmt::format(fmt, args...));
            logger.MessageNow(level, uMod, strMessage.c_str(), strMessage.size());
        }
    }
    catch (const fmt::format_error& e)
    {
        logger.Print(LogLevel::FAULT, e.what());
        assert(false && "Format log message failed, please check your format and params!");
    }
    catch (const std::exception& e)
    {
        (void)e;
        assert(false && "Print/Write log message failed!");
    }
}
}  // namespace tracejj

#ifndef STRINGIZE
#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)
#endif

#ifndef GOTO_PATH
#ifdef _WIN32
#define GOTO_PATH ">"
#else
#define GOTO_PATH
#endif
#endif

#ifndef LINE_BREAK
#define LINE_BREAK "\n"
#endif

// LOC: line of code
#define LOG_CRIT_LOC(fmt, ...) tracejj::LogPrintAndWrite(tracejj::LogLevel::CRIT, 0u, fmt LINE_BREAK GOTO_PATH __FILE__ "(" STRINGIZE(__LINE__) ", 0)" LINE_BREAK, ##__VA_ARGS__)
#define LOG_ERROR_LOC(fmt, ...) tracejj::LogPrintAndWrite(tracejj::LogLevel::FAULT, 0u, fmt LINE_BREAK GOTO_PATH __FILE__ "(" STRINGIZE(__LINE__) ", 0)" LINE_BREAK, ##__VA_ARGS__)
#define LOG_WARNING_LOC(fmt, ...) tracejj::LogPrintAndWrite(tracejj::LogLevel::WARN, 0u, fmt LINE_BREAK GOTO_PATH __FILE__ "(" STRINGIZE(__LINE__) ", 0)" LINE_BREAK, ##__VA_ARGS__)
#define LOG_NOTICE_LOC(fmt, ...) tracejj::LogPrintAndWrite(tracejj::LogLevel::NOTICE, 0u, fmt LINE_BREAK GOTO_PATH __FILE__ "(" STRINGIZE(__LINE__) ", 0)" LINE_BREAK, ##__VA_ARGS__)
#define LOG_INFO_LOC(fmt, ...) tracejj::LogPrintAndWrite(tracejj::LogLevel::INFO, 0u, fmt LINE_BREAK GOTO_PATH __FILE__ "(" STRINGIZE(__LINE__) ", 0)" LINE_BREAK, ##__VA_ARGS__)
#define LOG_DEBUG_LOC(fmt, ...) tracejj::LogPrintAndWrite(tracejj::LogLevel::DEBUG, 0u, fmt LINE_BREAK GOTO_PATH __FILE__ "(" STRINGIZE(__LINE__) ", 0)" LINE_BREAK, ##__VA_ARGS__)

#define LOG_CRITICAL(fmt, ...) LOG_CRIT_LOC(fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG_ERROR_LOC(fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) tracejj::LogPrintAndWrite(tracejj::LogLevel::WARN, 0u, fmt LINE_BREAK, ##__VA_ARGS__)
#define LOG_NOTICE(fmt, ...) tracejj::LogPrintAndWrite(tracejj::LogLevel::NOTICE, 0u, fmt LINE_BREAK, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) tracejj::LogPrintAndWrite(tracejj::LogLevel::INFO, 0u, fmt LINE_BREAK, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) tracejj::LogPrintAndWrite(tracejj::LogLevel::DEBUG, 0u, fmt LINE_BREAK, ##__VA_ARGS__)
