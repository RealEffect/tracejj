#include "System.h"

#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#else
#include <unistd.h>
#include <pthread.h>
#define _getpid getpid
#endif

#ifdef __linux__
#include <sys/syscall.h>
#ifndef SYS_gettid
#error "SYS_gettid unavailable on this system"
#endif
#define gettid() ((pid_t)syscall(SYS_gettid))
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
#else
    return gettid();
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
    static const int s_nProcessNumber = _getpid();
    return s_nProcessNumber;
}

void SetThreadAlias(const char* name)
{
    if (name != nullptr)
    {
#ifdef __linux__
        pthread_setname_np(pthread_self(), name);
#endif
    }
}

}  // namespace tracejj
