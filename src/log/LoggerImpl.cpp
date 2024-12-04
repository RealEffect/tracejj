#include "LoggerImpl.h"
#include "platform/System.h"
#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/chrono.h>
#include <fmt/xchar.h>
#include <cassert>
#include <cstdio>
#if defined(WIN32)
#include <Windows.h>
#include <io.h>
#else
#include <unistd.h>
#define _isatty isatty
#define _fileno fileno
#endif

//////////////////////////////////////////////////////////////////////////
namespace fmt
{
using namespace tracejj;

template <>
struct formatter<LogLevel>
{
    constexpr auto parse(format_parse_context& ctx) const -> decltype(ctx.begin())
    {
        return ctx.begin();
    }

    const char* strEmpty = "";  // for fix https://stackoverflow.com/questions/67344216

    template <typename FormatContext>
    auto format(const LogLevel& object, FormatContext& ctx) const -> decltype(format_to(ctx.out(), strEmpty))
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
        default: return format_to(ctx.out(), "{}", static_cast<uint32_t>(object));
        }
    }
};
}  // namespace fmt

//////////////////////////////////////////////////////////////////////////
namespace tracejj
{

Logger& Logger::Instance()
{
    static LoggerImpl s_instance;
    return s_instance;
}

LoggerImpl::LoggerImpl()
    : m_levelMax(LogLevel::MAX)
    , m_enableGenPrefix(true)
    , m_enablePrint2Stdout(true)
    , m_enablePrint2Debug(true)
    , m_arrWriter({nullptr})
    , m_pPrintObserver(nullptr)
{
}

LoggerImpl::~LoggerImpl()
{
    for (auto& item : m_arrWriter)
    {
        item = nullptr;
    }
    m_pPrintObserver = nullptr;
}

bool LoggerImpl::SetWriter(uint32_t uMod, LogWriter* pWriter)
{
    if (uMod >= static_cast<uint32_t>(m_arrWriter.size()))
    {
        return false;
    }
    m_arrWriter[uMod] = pWriter;
    return true;
}

void LoggerImpl::SetMaxLevel(LogLevel level)
{
    m_levelMax = level;
}

void LoggerImpl::SetGeneratePrefix(bool gen)
{
    m_enableGenPrefix = gen;
}

void LoggerImpl::SetPrint2Stdout(bool enable)
{
    m_enablePrint2Stdout = enable;
}

void LoggerImpl::SetPrint2DebugWindow(bool enable)
{
    m_enablePrint2Debug = enable;
}

void LoggerImpl::SetPrintObserver(Observer* pObserver)
{
    m_pPrintObserver = pObserver;
}

LogLevel LoggerImpl::MaxLevel() const
{
    return m_levelMax;
}

void LoggerImpl::MessageNow(LogLevel level, uint32_t uMod, const char* strMessage, size_t szMessage)
{
    if (level > m_levelMax)
    {
        return;
    }
    if (m_enableGenPrefix)
    {
        const auto tickNow = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() / double(std::micro::den);
        const auto timeNow = std::chrono::system_clock::now();
        const auto timeNowSecond = std::chrono::time_point_cast<std::chrono::seconds>(timeNow);
        const auto timeNowSubsecond = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - timeNowSecond);
        const auto tmNow = fmt::localtime(std::chrono::system_clock::to_time_t(timeNowSecond));
        const auto strFormat = fmt::format("{:%F %T}.{:03} [{}@{:.6f}]<{}>: {}", tmNow, timeNowSubsecond.count(), CurrentThreadNumber(), tickNow, level, fmt::string_view(strMessage, szMessage));
        Write(level, uMod, strFormat.c_str(), strFormat.size());
        Print(level, strFormat.c_str(), strFormat.size());
    }
    else
    {
        Write(level, uMod, strMessage, szMessage);
        Print(level, strMessage, szMessage);
    }
}

void LoggerImpl::Print(LogLevel level, const char* strMessage)
{
    if (static_cast<uint32_t>(level) > static_cast<uint32_t>(m_levelMax))
    {
        return;
    }
    Print(level, strMessage, strlen(strMessage));
}

void LoggerImpl::Print(LogLevel level, const char* strMessage, size_t szMessage)
{
    if (static_cast<uint32_t>(level) > static_cast<uint32_t>(m_levelMax))
    {
        return;
    }
#ifdef _WIN32
    if (m_enablePrint2Debug && ::IsDebuggerPresent())
    {
        ::OutputDebugStringA(strMessage);
    }
#endif
    if (m_enablePrint2Stdout && _isatty(_fileno(stdout)) != 0)
    {
        switch (level)
        {
        case LogLevel::FATAL: fmt::print(fg(fmt::color::crimson), "{}", fmt::string_view(strMessage, szMessage)); break;
        case LogLevel::FAULT: fmt::print(fg(fmt::color::crimson), "{}", fmt::string_view(strMessage, szMessage)); break;
        case LogLevel::WARN: fmt::print(fg(fmt::color::yellow), "{}", fmt::string_view(strMessage, szMessage)); break;
        case LogLevel::NOTICE: fmt::print(fg(fmt::color::green), "{}", fmt::string_view(strMessage, szMessage)); break;
        case LogLevel::INFO: fmt::print(fg(fmt::color::white), "{}", fmt::string_view(strMessage, szMessage)); break;
        case LogLevel::DEBUG: fmt::print("{}", fmt::string_view(strMessage, szMessage)); break;
        default: fmt::print("{}", fmt::string_view(strMessage, szMessage)); break;
        }
    }
    if (m_pPrintObserver)
    {
        m_pPrintObserver->OnMessage(level, strMessage, szMessage);
    }
}

void LoggerImpl::Write(LogLevel level, uint32_t uMod, const char* strMessage)
{
    if (static_cast<uint32_t>(level) > static_cast<uint32_t>(m_levelMax))
    {
        return;
    }
    Write(level, uMod, strMessage, strlen(strMessage));
}

void LoggerImpl::Write(LogLevel level, uint32_t uMod, const char* strMessage, size_t szMessage)
{
    if (static_cast<uint32_t>(level) > static_cast<uint32_t>(m_levelMax))
    {
        return;
    }
    if (uMod >= static_cast<uint32_t>(m_arrWriter.size()))
    {
        return;
    }
    if (m_arrWriter[uMod] != nullptr)
    {
        m_arrWriter[uMod]->Write(level, strMessage, szMessage);
    }
    else
    {
        if (m_arrWriter[0] != nullptr)
        {
            m_arrWriter[0]->Write(level, strMessage, szMessage);
        }
    }
}

}  // namespace tracejj
