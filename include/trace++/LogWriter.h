#pragma once

#include <cstdint>
#include <cstddef>

#include "trace++/LibDefine.h"
#include "trace++/System.h"

// see syslog level
enum class LogLevel : uint32_t
{
    FATAL = 0U,  // or Name it 'Emergency'
    ALERT = 1U,
    CRIT = 2U,
    FAULT = 3U,  // or Name it 'Error'
    WARN = 4U,
    NOTICE = 5U,
    INFO = 6U,
    DEBUG = 7U,  // or Name it 'Verbose'
    MAX = 0xFFFFU,
};

class LogWriter
{
public:
    struct Param
    {
        const tracejj::pchar_t* strLogDir = nullptr;
        uint32_t uMaxLogFileSize = 0U;  // MiB
        uint32_t uMaxLogStorage = 0U;   // MiB
    };

public:
    TRACE_API static LogWriter* AllocFileWriter(const Param* pParam);

public:
    LogWriter() = default;
    virtual ~LogWriter() = default;

public:
    virtual bool Write(LogLevel level, const char* strMsg, size_t szMsgLength) = 0;
    virtual void Close() = 0;
};