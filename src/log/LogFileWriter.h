#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "trace++/LogWriter.h"
#include "buffer/MultiWriteRingBuffer.h"
#include "platform/Path.h"

namespace tracejj
{
class LogFileWriter final : public LogWriter
{
private:
    struct LogBuffer
    {
        MultiWriteRingBuffer data;
        std::atomic<uint32_t> loss;
        uint32_t flags;
        LogBuffer()
            : data(32ull * 1024ull)
            , loss(0u)
            , flags(0u)
        {
        }
    };

public:
    LogFileWriter();
    LogFileWriter(const LogWriter::Param* pParam);
    LogFileWriter(const LogFileWriter&) noexcept = delete;
    LogFileWriter(LogFileWriter&&) noexcept = delete;
    ~LogFileWriter() override;

public:
    LogFileWriter& operator=(const LogFileWriter&) noexcept = delete;
    LogFileWriter& operator=(LogFileWriter&&) noexcept = delete;

public:
    bool Write(LogLevel level, const char* strMsg, size_t szMsgLength) override;

private:
    void Start(const path_string& strDir);
    void Stop();
    int OpenLogFile(const path_string& strDir);
    void LoopWrite(const path_string& strDir);
    void PruneLogStorage(bool& bBreak, const path_string& strDir);

private:
    volatile bool m_bLogging;
    uint64_t m_uMaxLogFileSize;
    uint64_t m_uMaxLogStorage;
    std::thread m_thWrite;
    LogBuffer m_buffer;
    std::mutex m_mtxStatus;
    std::condition_variable m_cvWriteEvent;
};
}  // namespace tracejj
