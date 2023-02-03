#pragma once

#ifdef TRACEJJ_BUILD_FOR_SHARED
#if __GNUC__ >= 4
#define TRACE_API __attribute__((visibility("default")))
#elif defined(_MSC_VER)
#if defined(tracejj_EXPORTS)
#define TRACE_API __declspec(dllexport)
#else
#define TRACE_API __declspec(dllimport)
#endif
#else
#define TRACE_API
#endif
#else
#define TRACE_API
#endif
