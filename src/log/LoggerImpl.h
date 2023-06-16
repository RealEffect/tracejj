#pragma once

#include "trace++/Logger.h"
#include <array>
#include <shared_mutex>
#include <string>

#ifndef MAX_LOG_MODS
#define MAX_LOG_MODS (16)
#endif

class LoggerImpl : public Logger
{
public:
    LoggerImpl();
    LoggerImpl(const LoggerImpl&) noexcept = delete;
    LoggerImpl(const LoggerImpl&&) noexcept = delete;

private:
    ~LoggerImpl() override;

public:
    LoggerImpl& operator=(const LoggerImpl&) noexcept = delete;
    LoggerImpl& operator=(LoggerImpl&&) noexcept = delete;

public:
    bool SetWriter(uint32_t uMod, LogWriter* pWriter) override;
    void SetMaxLevel(LogLevel level) override;
    void SetPrint2Stdout(bool enable) override;
    void SetPrint2DebugWindow(bool enable) override;
    void SetPrintObserver(Observer* pObserver) override;
    [[nodiscard]] LogLevel MaxLevel() const override;
    void Print(LogLevel level, const char* strMessage) override;
    void Print(LogLevel level, const char* strMessage, size_t szMessage) override;
    void Write(LogLevel level, uint32_t uMod, const char* strMessage) override;
    void Write(LogLevel level, uint32_t uMod, const char* strMessage, size_t szMessage) override;

private:
    void Print(LogLevel level, const std::string_view& strMessage) const;
    void Write(LogLevel level, uint32_t uMod, const std::string_view& strMessage);

private:
    LogLevel m_levelMax;
    bool m_enablePrint2Stdout;
    bool m_enablePrint2DebugWindow;
    std::shared_mutex m_mtxWriter;
    std::array<LogWriter*, MAX_LOG_MODS> m_arrWriter;
    Observer* m_pPrintObserver;
};
