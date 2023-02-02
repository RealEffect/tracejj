#pragma once

#include "trace++/LibDefine.h"
#include <stdint.h>

namespace tracejj
{
/// char type of path
#ifdef _WIN32
typedef wchar_t pchar_t;
#else
typedef char pchar_t;
#endif

TRACE_API uint32_t CurrentThreadNumber();
TRACE_API int CurrentProcessNumber();
}  // namespace tracejj
