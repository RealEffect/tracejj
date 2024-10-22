#pragma once

#include "trace++/LogWriter.h"
#include "trace++/LibDefine.h"

namespace tracejj
{
class Logger
{
public:
    TRACE_API static Logger& Instance();

public:
    class Observer
    {
    protected:
        Observer() = default;
        virtual ~Observer() = default;

    public:
        virtual void OnMessage(LogLevel level, const char* strMsg, size_t szMsg) = 0;
    };

protected:
    Logger() = default;
    virtual ~Logger() = default;

public:
    virtual bool SetWriter(uint32_t uMod, LogWriter* pWriter) = 0;
    virtual void SetMaxLevel(LogLevel level) = 0;
    virtual void SetGeneratePrefix(bool gen) = 0;
    virtual void SetPrint2Stdout(bool enable) = 0;
    virtual void SetPrint2DebugWindow(bool enable) = 0;
    virtual void SetPrintObserver(Observer* pObserver) = 0;
    virtual LogLevel MaxLevel() const = 0;

public:
    virtual void MessageNow(LogLevel level, uint32_t uMod, const char* strMessage, size_t szMessage) = 0;
    virtual void Print(LogLevel level, const char* strMessage) = 0;
    virtual void Print(LogLevel level, const char* strMessage, size_t szMessage) = 0;
    virtual void Write(LogLevel level, uint32_t uMod, const char* strMessage) = 0;
    virtual void Write(LogLevel level, uint32_t uMod, const char* strMessage, size_t szMessage) = 0;
};
}  // namespace tracejj
