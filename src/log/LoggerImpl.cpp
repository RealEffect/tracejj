#include "log/LoggerImpl.h"
#include <fmt/core.h>
#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#endif

// todo, thread safe! leak!
namespace tracepp_logger_impl_internal
{
static LoggerImpl* s_pInstance = new LoggerImpl();
}

Logger& Logger::Instance()
{
    using namespace tracepp_logger_impl_internal;
    return *s_pInstance;
}

LoggerImpl::LoggerImpl()
    : m_levelMax(LogLevel::MAX)
    , m_enablePrint2Stdout(true)
    , m_enablePrint2DebugWindow(true)
    , m_arrWriter({nullptr})
    , m_pPrintObserver(nullptr)
{
}

LoggerImpl::~LoggerImpl()
{
    m_pPrintObserver = nullptr;
}

bool LoggerImpl::SetWriter(uint32_t uMod, LogWriter* pWriter)
{
    if (uMod > static_cast<uint32_t>(m_arrWriter.size()))
    {
        return false;
    }
    std::lock_guard<std::shared_mutex> lock(m_mtxWriter);
    m_arrWriter[uMod] = pWriter;
    return true;
}

void LoggerImpl::SetMaxLevel(LogLevel level)
{
    m_levelMax = level;
}

void LoggerImpl::SetPrint2Stdout(bool enable)
{
    m_enablePrint2Stdout = enable;
}

void LoggerImpl::SetPrint2DebugWindow(bool enable)
{
    m_enablePrint2DebugWindow = enable;
}

void LoggerImpl::SetPrintObserver(Observer* pObserver)
{
    m_pPrintObserver = pObserver;
}

LogLevel LoggerImpl::MaxLevel() const
{
    return m_levelMax;
}

void LoggerImpl::Print(LogLevel level, const char* strMessage)
{
    if (static_cast<uint32_t>(level) > static_cast<uint32_t>(m_levelMax))
    {
        return;
    }
    Print(level, std::string_view(strMessage, strlen(strMessage)));
}

void LoggerImpl::Print(LogLevel level, const char* strMessage, size_t szMessage)
{
    if (static_cast<uint32_t>(level) > static_cast<uint32_t>(m_levelMax))
    {
        return;
    }
    Print(level, std::string_view(strMessage, szMessage));
}

void LoggerImpl::Write(LogLevel level, uint32_t uMod, const char* strMessage)
{
    if (static_cast<uint32_t>(level) > static_cast<uint32_t>(m_levelMax))
    {
        return;
    }
    Write(level, uMod, std::string_view(strMessage, strlen(strMessage)));
}

void LoggerImpl::Write(LogLevel level, uint32_t uMod, const char* strMessage, size_t szMessage)
{
    if (static_cast<uint32_t>(level) > static_cast<uint32_t>(m_levelMax))
    {
        return;
    }
    Write(level, uMod, std::string_view(strMessage, szMessage));
}

void LoggerImpl::Print(LogLevel level, const std::string_view& strMessage) const
{
#ifdef _WIN32
    if (m_enablePrint2DebugWindow && ::IsDebuggerPresent())
    {
        ::OutputDebugStringA(strMessage.data());
    }
#endif
    if (m_enablePrint2Stdout)
    {
        std::cout << strMessage;
    }
    if (m_pPrintObserver)
    {
        m_pPrintObserver->OnMessage(level, strMessage.data(), strMessage.size());
    }
}

void LoggerImpl::Write(LogLevel level, uint32_t uMod, const std::string_view& strMessage)
{
    if (uMod > static_cast<uint32_t>(m_arrWriter.size()))
    {
        return;
    }
    std::shared_lock<std::shared_mutex> lock(m_mtxWriter);
    if (m_arrWriter[uMod] != nullptr)
    {
        m_arrWriter[uMod]->Write(level, strMessage.data(), strMessage.size());
    }
    else
    {
        if (m_arrWriter[0] != nullptr)
        {
            m_arrWriter[0]->Write(level, strMessage.data(), strMessage.size());
        }
    }
}
