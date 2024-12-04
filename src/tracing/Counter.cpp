#include "Counter.h"
#include "trace++/Logging.hpp"
#include "platform/Path.h"
#include "platform/System.h"

#if __linux__
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace tracejj
{
class TrackFIFO
{
public:
    TrackFIFO(const std::string& strName)
        : m_fp(-1)
        , m_fd(-1)
    {
        if (!strName.empty())
        {
            fs::path pathFIFO("/run");
            pathFIFO /= GetExecutablePath().filename();
            std::error_code ec;
            if (!fs::is_directory(pathFIFO, ec))
            {
                if (!fs::create_directories(pathFIFO, ec))
                {
                    return;
                }
            }
            pathFIFO /= strName;
            m_strPath = pathFIFO.native();
            ::mkfifo(m_strPath.c_str(), 0644);
            m_fp = ::open(m_strPath.c_str(), O_RDONLY | O_NONBLOCK);
            if (m_fp >= 0)
            {
                m_fd = ::open(m_strPath.c_str(), O_WRONLY | O_NONBLOCK);
            }
        }
    }
    ~TrackFIFO()
    {
        if (m_fd >= 0)
        {
            ::close(m_fd);
            m_fd = -1;
        }
        if (m_fp >= 0)
        {
            ::close(m_fp);
            m_fp = -1;
        }
    }

public:
    TrackFIFO& operator<<(const std::string& strMsg)
    {
        if (m_fd >= 0)
        {
            if (::write(m_fd, strMsg.c_str(), strMsg.size()) < 0 && errno == EAGAIN)
            {
                char buffer[2048];
                ::read(m_fp, buffer, sizeof(buffer));
                ::write(m_fd, strMsg.c_str(), strMsg.size());
            }
        }
        return *this;
    }

private:
    int m_fp;
    int m_fd;
    std::string m_strPath;
};
}  // namespace tracejj

#else

namespace tracejj
{
class TrackFIFO
{
public:
    TrackFIFO(const std::string&) = default;
    ~TrackFIFO() = default;

public:
    TrackFIFO& operator<<(const std::string&)
    {
        return *this;
    }
};
}  // namespace tracejj

#endif

using namespace std::chrono;
using namespace std::chrono_literals;

namespace tracejj
{
//////////////////////////////////////////////////////////////////////////
Counter* Counter::NewP1S(const char* strName)
{
    return new P1SCounter(strName);
}

//////////////////////////////////////////////////////////////////////////
P1SCounter::P1SCounter(const char* name)
    : m_strName(name == nullptr ? "" : name)
    , m_duMin(0)
    , m_duMax(0)
    , m_uTotal(0ull)
    , m_uTrace(0ull)
    , m_pFIFO(nullptr)
{
    if (!m_strName.empty())
    {
        m_pFIFO = new TrackFIFO(m_strName);
    }
}

P1SCounter::~P1SCounter()
{
    if (m_pFIFO)
    {
        delete m_pFIFO;
        m_pFIFO = nullptr;
    }
}

void P1SCounter::Pace(uint64_t uInc, uint32_t uLogMod)
{
    const auto now = steady_clock::now();
    if (m_uTotal == 0ull)
    {
        m_tpStart = now;
        m_tpTrace = now;
        m_tpLast = now;
        m_uTrace += uInc;
        m_uTotal += uInc;
    }
    else
    {
        const auto duPace = now - m_tpLast;
        m_tpLast = now;
        const auto duTrace = now - time_point_cast<milliseconds>(m_tpTrace);
        if (duTrace >= 999ms)
        {
            const auto dTotal = duration_cast<milliseconds>(now - m_tpStart).count() / double(std::milli::den);
            const auto dTrace = duration_cast<milliseconds>(duTrace).count() / double(std::milli::den);
            if (m_pFIFO)
            {
                const auto tickNow = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() / double(std::micro::den);
                *m_pFIFO << fmt::format("[{}:{}@{:0.6f}] TOTAL={}@{}s TRACE={}@{}s MIN={} MAX={}\n", CurrentProcessNumber(), CurrentThreadNumber(), tickNow, m_uTotal, dTotal, m_uTrace, dTrace,
                                        duration_cast<milliseconds>(m_duMin), duration_cast<milliseconds>(m_duMax));
            }
            if (!m_strName.empty() && uLogMod < UINT32_MAX)
            {
                LogPrintAndWrite(LogLevel::INFO, uLogMod, "%{} TOTAL={}@{}s TRACE={}@{}s MIN={} MAX={}\n", m_strName, m_uTotal, dTotal, m_uTrace, dTrace, duration_cast<milliseconds>(m_duMin),
                                 duration_cast<milliseconds>(m_duMax));
            }
            m_tpTrace = now;
            m_duMin = duPace;
            m_duMax = m_duMin;
            m_uTrace = uInc;
        }
        else
        {
            if (m_uTotal == 2u || m_duMin > duPace)
            {
                m_duMin = duPace;
            }
            if (m_uTotal == 2u || m_duMax < duPace)
            {
                m_duMax = duPace;
            }
            m_uTrace += uInc;
        }
        m_uTotal += uInc;
    }
}
}  // namespace tracejj
