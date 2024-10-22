#pragma once

#include <cstdint>
#include <cstddef>

#include "trace++/LibDefine.h"

namespace tracejj
{
// see syslog level
enum class LogLevel : uint32_t
{
    FATAL = 0u,  // or Name it 'Emergency'
    ALERT = 1u,
    CRIT = 2u,
    FAULT = 3u,  // or Name it 'Error'
    WARN = 4u,
    NOTICE = 5u,
    INFO = 6u,
    DEBUG = 7u,  // or Name it 'Verbose'
    MAX = 0xFFFFu,
};

class LogWriter
{
public:
    struct Param
    {
        const tracejj::pchar_t* strLogDir = nullptr;
        uint32_t uMaxLogFileSize = 0u;  // MiB
        uint32_t uMaxLogStorage = 0u;   // MiB
    };

public:
    TRACE_API static LogWriter* AllocFileWriter(const Param* pParam);

public:
    LogWriter() = default;
    virtual ~LogWriter() = default;

public:
    virtual bool Write(LogLevel level, const char* strMsg, size_t szMsgLength) = 0;
};
}  // namespace tracejj
