#pragma once
#include <trace++/LibDefine.h>
#include <cstdint>

namespace tracejj
{

class Counter
{
public:
    TRACE_API static Counter* NewP1S(const char* strName);

public:
    Counter() = default;
    virtual ~Counter() = default;

public:
    virtual void Pace(uint64_t uInc, uint32_t uLogMod) = 0;
    inline void Pace(uint64_t uInc)
    {
        Pace(uInc, UINT32_MAX);
    }
};

}  // namespace tracejj