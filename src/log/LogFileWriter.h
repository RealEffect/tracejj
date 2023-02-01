#pragma once

#include "trace++/LogWriter.h"
#include "buffer/MultiWriteRingBuffer.h"
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

#ifdef _WIN32
typedef std::wstring path_string;
#else
typedef std::string path_string;
#endif

class LogFileWriter final : public LogWriter
{
    struct LogBuffer
    {
        MultiWriteRingBuffer data;
        std::atomic_uint32_t loss;
        uint32_t flags;
        LogBuffer()
            : data(32U * 1024U)
            , loss(0U)
            , flags(0U)
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
    void Close() override;

private:
    void Start(const path_string& strDir);
    int OpenLogFile(const path_string& strDir);
    void LoopWrite(path_string strDir);
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