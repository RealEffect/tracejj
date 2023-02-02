#include "trace++/System.h"

#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#define getpid _getpid
#else
#include <unistd.h>
#include <pthread.h>
#endif

#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#else
#define FORCE_INLINE __attribute__((__always_inline__))
#endif

namespace tracejj
{

uint32_t ThisThreadId()
{
#if defined(WIN32)
    return ::GetCurrentThreadId();
#elif defined(__APPLE__)
    return pthread_mach_thread_np(pthread_self());
#elif defined(__linux__)
    return pthread_self();
#else
    // todo(other OS)
#endif
}

class Thread
{
public:
    Thread()
        : m_uNumber(ThisThreadId())
    {
    }

public:
    FORCE_INLINE uint32_t Number() const
    {
        return m_uNumber;
    }

private:
    const uint32_t m_uNumber;
};

//////////////////////////////////////////////////////////////////////////
uint32_t CurrentThreadNumber()
{
    thread_local Thread t_self;
    return t_self.Number();
}

int CurrentProcessNumber()
{
    static const int s_nProcessNumber = getpid();
    return s_nProcessNumber;
}

}  // namespace tracejj
