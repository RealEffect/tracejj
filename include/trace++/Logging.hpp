#pragma once

#include <cstdint>
#include <cassert>
#include <fmt/core.h>
#include <fmt/chrono.h>
#include <fmt/ostream.h>

#include "trace++/Logger.h"
#include "trace++/System.h"

template <>
struct fmt::formatter<LogLevel>
{
    constexpr auto parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const LogLevel& object, FormatContext& ctx)
    {
        switch (object)
        {
        case LogLevel::FATAL: return format_to(ctx.out(), "FATAL!");
        case LogLevel::CRIT: return format_to(ctx.out(), "CRITICAL");
        case LogLevel::FAULT: return format_to(ctx.out(), "ERROR");
        case LogLevel::WARN: return format_to(ctx.out(), "WARN");
        case LogLevel::NOTICE: return format_to(ctx.out(), "NOTICE");
        case LogLevel::INFO: return format_to(ctx.out(), "INFO");
        case LogLevel::DEBUG: return format_to(ctx.out(), "DEBUG");
        default: return format_to(ctx.out(), "$_$");
        }
    }
};

inline std::string SetupLogMessageHeader(LogLevel level, const std::string& strMessage)
{
    auto tickNow = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() / 1000000.0;
    auto timeNow = std::chrono::system_clock::now();
    auto timeNowFreq = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow.time_since_epoch()).count() % 1000;
    return fmt::format("{}.{:03} [{}@{:.6f}]<{}>: {}", timeNow, timeNowFreq, tracepp::CurrentThreadNumber(), tickNow, level, strMessage);
}

template <typename... Args>
inline void LogPrintAndWrite(LogLevel level, int32_t nMod, const char* fmt, Args&&... args)
{
    try
    {
        auto& logger = Logger::Instance();
        if (static_cast<uint32_t>(logger.MaxLevel()) >= static_cast<uint32_t>(level))
        {
            std::string strLog(std::move(SetupLogMessageHeader(level, fmt::format(fmt, args...))));
            logger.Write(level, nMod, strLog.c_str(), strLog.length());
            logger.Print(level, strLog.c_str(), strLog.length());
        }
    }
    catch (const fmt::format_error& e)
    {
        auto& logger = Logger::Instance();
        logger.Print(LogLevel::FAULT, e.what());
        assert(false && "Format log message failed! please check your format and params.");
    }
    catch (const std::exception& e)
    {
        (void)e;
        assert(false && "Print/Write log message failed!");
    }
}

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
#define LINE_BREAK "\r\n"
#endif

// LOC: line of code
#define LOG_CRIT_LOC(fmt, ...) LogPrintAndWrite(LogLevel::CRIT, 0, fmt LINE_BREAK GOTO_PATH __FILE__ "(" STRINGIZE(__LINE__) ", 0)" LINE_BREAK, ##__VA_ARGS__)
#define LOG_ERROR_LOC(fmt, ...) LogPrintAndWrite(LogLevel::FAULT, 0, fmt LINE_BREAK GOTO_PATH __FILE__ "(" STRINGIZE(__LINE__) ", 0)" LINE_BREAK, ##__VA_ARGS__)
#define LOG_WARNING_LOC(fmt, ...) LogPrintAndWrite(LogLevel::WARN, 0, fmt LINE_BREAK GOTO_PATH __FILE__ "(" STRINGIZE(__LINE__) ", 0)" LINE_BREAK, ##__VA_ARGS__)
#define LOG_NOTICE_LOC(fmt, ...) LogPrintAndWrite(LogLevel::NOTICE, 0, fmt LINE_BREAK GOTO_PATH __FILE__ "(" STRINGIZE(__LINE__) ", 0)" LINE_BREAK, ##__VA_ARGS__)
#define LOG_INFO_LOC(fmt, ...) LogPrintAndWrite(LogLevel::INFO, 0, fmt LINE_BREAK GOTO_PATH __FILE__ "(" STRINGIZE(__LINE__) ", 0)" LINE_BREAK, ##__VA_ARGS__)
#define LOG_DEBUG_LOC(fmt, ...) LogPrintAndWrite(LogLevel::DEBUG, 0, fmt LINE_BREAK GOTO_PATH __FILE__ "(" STRINGIZE(__LINE__) ", 0)" LINE_BREAK, ##__VA_ARGS__)

#define LOG_CRITICAL(fmt, ...) LOG_CRIT_LOC(fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG_ERROR_LOC(fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) LogPrintAndWrite(LogLevel::WARN, 0, fmt LINE_BREAK, ##__VA_ARGS__)
#define LOG_NOTICE(fmt, ...) LogPrintAndWrite(LogLevel::NOTICE, 0, fmt LINE_BREAK, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) LogPrintAndWrite(LogLevel::INFO, 0, fmt LINE_BREAK, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) LogPrintAndWrite(LogLevel::DEBUG, 0, fmt LINE_BREAK, ##__VA_ARGS__)
