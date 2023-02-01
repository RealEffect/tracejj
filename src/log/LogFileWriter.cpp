#include "log/LogFileWriter.h"
#include "trace++/System.h"
#include "system/Path.h"
#include <fmt/format.h>
#include <fmt/chrono.h>
#include <future>
#include <list>

#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#define write _write
#define close _close
#else
#include <unistd.h>
#endif
#include <fcntl.h>

#define DEFAULT_MAX_FILE_SIZE (16ull * 1024u * 1024u)  // 16MB
#define FLAG_LOG_FLUSH (0x00000001)
#define FLAG_LOG_EOF (0x00000002)
#define CHECK_FLAG(flags, mark) ((flags & mark) == mark)

constexpr size_t kWriteCacheSize = 2ULL * 1024ULL;

//////////////////////////////////////////////////////////////////////////
LogWriter* LogWriter::AllocFileWriter(const Param* pParam)
{
    return new LogFileWriter(pParam);
}

LogFileWriter::LogFileWriter()
    : m_bLogging(true)
    , m_uMaxLogFileSize(DEFAULT_MAX_FILE_SIZE)
    , m_uMaxLogStorage(8ULL * DEFAULT_MAX_FILE_SIZE)
{
    Start(path_string());
}

LogFileWriter::LogFileWriter(const LogWriter::Param* pParam)
    : m_bLogging(true)
    , m_uMaxLogFileSize(DEFAULT_MAX_FILE_SIZE)
    , m_uMaxLogStorage(8ull * DEFAULT_MAX_FILE_SIZE)
{
    path_string strLogDir;
    if (pParam)
    {
        if (pParam->strLogDir)
        {
            strLogDir.assign(pParam->strLogDir);
        }
        if (pParam->uMaxLogFileSize > 0U)
        {
            m_uMaxLogFileSize = static_cast<uint64_t>(pParam->uMaxLogFileSize) << 20;
        }
        if (pParam->uMaxLogStorage > 0U && pParam->uMaxLogStorage > pParam->uMaxLogFileSize)
        {
            m_uMaxLogStorage = static_cast<uint64_t>(pParam->uMaxLogStorage) << 20;
        }
    }
    Start(strLogDir);
}

LogFileWriter::~LogFileWriter()
{
    LogFileWriter::Close();
}

bool LogFileWriter::Write(LogLevel level, const char* strMsg, size_t szMsgLength)
{
    if (strMsg == nullptr || szMsgLength == 0 || !m_bLogging)
    {
        return false;
    }
    else
    {
        if (m_buffer.data.Write(reinterpret_cast<const int8_t*>(strMsg), static_cast<uint32_t>(szMsgLength)))
        {
            if (level <= LogLevel::FAULT || (m_buffer.data.Bytes() > kWriteCacheSize && !CHECK_FLAG(m_buffer.flags, FLAG_LOG_FLUSH)))
            {
                std::lock_guard<std::mutex> lock(m_mtxStatus);
                m_buffer.flags |= FLAG_LOG_FLUSH;
                m_cvWriteEvent.notify_all();
            }
            return true;
        }
        else
        {
            ++m_buffer.loss;
            return false;
        }
    }
}

void LogFileWriter::Close()
{
    if (m_thWrite.joinable())
    {
        if (m_buffer.data.Empty())
        {
            // 关闭时无日志写入，直接关闭写入。
            m_bLogging = false;
        }
        else
        {
            // 关闭时如果还有日志未写入文件，设置EOF
            // 同时等待日志写入完成后关闭。
            // 避免进程结束时最后的部分日志丢失。
            std::lock_guard<std::mutex> lock(m_mtxStatus);
            m_buffer.flags |= FLAG_LOG_EOF;
        }
        m_cvWriteEvent.notify_all();
        m_thWrite.join();
    }
}

void LogFileWriter::Start(const path_string& strDir)
{
    fs::path pathLog(strDir);
    if (pathLog.empty())
    {
        pathLog = GetLogsPath();
        pathLog.append("log");
    }
    std::error_code ec;
    if (!fs::is_directory(pathLog, ec))
    {
        if (!fs::create_directories(pathLog, ec))
        {
            return;
        }
    }
    m_thWrite = std::thread(&LogFileWriter::LoopWrite, this, pathLog.native());
}

int LogFileWriter::OpenLogFile(const path_string& strDir)
{
    try
    {
        fs::path pathLogFile(strDir);
        if (fs::is_directory(pathLogFile) || fs::create_directories(pathLogFile))
        {
            auto tick = std::chrono::system_clock::now();
            // log file name: <base name>.<system time>[#<conflict number>].<pid>.log
            // conflict number: 1 - 512
            // 日志文件名包含创建时间和进程号，参考glog，创建时间便于通过文件名筛选时间段，进程号
            // 可以兼容多进程，同时防止日志文件名冲突。
            // 加入文件名防冲突编号，1-512，兼容在短时间内（1秒内）大量日志输出导致文件滚动引起的文
            // 件名冲突。
            pathLogFile.append(fmt::format("{:%Y%m%d-%H%M%S}.{}.log", tick, tracepp::CurrentProcessNumber()));
            int nFileHandle = -1;
            for (int i = 1; i <= 512 && m_bLogging; ++i)
            {
#ifdef _WIN32
                const auto err = _wsopen_s(&nFileHandle, pathLogFile.c_str(), O_CREAT | O_EXCL | O_BINARY | O_RDWR, SH_DENYNO, S_IWRITE | S_IREAD);
#else
                nFileHandle == open(pathLogFile.c_str(), O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
                const auto err = nFileHandle;
#endif
                if (err == EEXIST)
                {
                    pathLogFile.replace_filename(fmt::format("{:%Y%m%d-%H%M%S}#{}.{}.log", tick, i, tracepp::CurrentProcessNumber()));
                }
                else
                {
                    break;
                }
            }
            return nFileHandle;
        }
    }
    catch (const std::exception&)
    {
        return -1;
    }
    return -1;
}

void LogFileWriter::LoopWrite(path_string strDir)
{
    bool bPruning = false;
    bool bEof = false;
    std::future<void> ftPrune;
    while (m_bLogging && !bEof)
    {
        const int nLogFileHandle = OpenLogFile(strDir);
        if (nLogFileHandle >= 0)
        {
            size_t szWriteBytesToFile = 0;
            while (m_bLogging)
            {
                if (m_buffer.data.Empty())
                {
                    std::unique_lock<std::mutex> lock(m_mtxStatus);
                    m_cvWriteEvent.wait_for(lock, std::chrono::seconds(1),
                                            [&]() -> bool
                                            {
                                                return !m_bLogging || m_buffer.flags != 0U;
                                            });
                }
                else
                {
                    const auto b = m_buffer.data.Peek();
                    write(nLogFileHandle, b.pSlice0, b.uSlice0Bytes);
                    szWriteBytesToFile += b.uSlice0Bytes;
                    if (b.uSlice1Bytes > 0U)
                    {
                        write(nLogFileHandle, b.pSlice1, b.uSlice1Bytes);
                        szWriteBytesToFile += b.uSlice1Bytes;
                    }
                    m_buffer.data.Readed(b);
                    if (m_buffer.loss > 0)
                    {
                        std::string strLoss = fmt::format("\n!!!!Log Loss {} Block.\n", m_buffer.loss.exchange(0U));
                        write(nLogFileHandle, strLoss.c_str(), static_cast<unsigned int>(strLoss.length()));
                        szWriteBytesToFile += strLoss.length();
                    }
                    if (CHECK_FLAG(m_buffer.flags, FLAG_LOG_EOF))
                    {
                        bEof = true;
                        break;
                    }
                    m_buffer.flags = 0U;
                    if (szWriteBytesToFile >= m_uMaxLogFileSize)
                    {
                        if (!bPruning)
                        {
                            // 每次日志文件封闭后，做一次清理。
                            bPruning = true;
                            ftPrune = std::async(
                                [this, &bPruning, &strDir]()
                                {
                                    PruneLogStorage(bPruning, strDir);
                                    bPruning = false;
                                });
                        }
                        break;
                    }
                }
            }
            close(nLogFileHandle);
        }
        else
        {
            break;
        }
    }
    if (ftPrune.valid())
    {
        bPruning = false;
        ftPrune.wait();
    }
    m_bLogging = false;
}

void LogFileWriter::PruneLogStorage(bool& bBreak, const path_string& strDir)
{
    // 清理旧日志文件，释放存储空间。
    const fs::path pathLog(strDir);
    if (pathLog.empty() || !fs::is_directory(pathLog))
    {
        return;
    }
    std::list<std::pair<int64_t, fs::directory_entry>> listAllFile;
    uint64_t uAllLogFileSize = 0U;
    std::error_code err;
    const auto timenow = fs::file_time_type::clock::now();
    for (auto& entry : fs::directory_iterator(pathLog, err))
    {
        if (!entry.is_directory(err) && entry.path().extension().compare(".log") == 0)
        {
            uAllLogFileSize += entry.file_size(err);
            if (err.value() == 0)
            {
                auto time = entry.last_write_time(err);
                if (timenow < time)
                {
                    // 文件时间可能大于当前时间，
                    // 这种文件直接识别为较旧的日志文件。
                    listAllFile.emplace_back((time - timenow).count(), entry);
                }
                else
                {
                    listAllFile.emplace_back(time.time_since_epoch().count(), entry);
                }
            }
        }
    }
    // 遍历日志文件后根据文件修改时间排序，
    // 超过日志存储上限后先删除旧的日志文件。
    listAllFile.sort();
    for (const auto& item : listAllFile)
    {
        if (uAllLogFileSize > m_uMaxLogStorage)
        {
            uAllLogFileSize -= item.second.file_size(err);
            fs::remove(item.second.path(), err);
        }
        else
        {
            break;
        }
    }
}
