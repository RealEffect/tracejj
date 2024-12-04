#pragma once

#include "trace++/Tracing.h"
#include <string>
#include <chrono>

namespace tracejj
{

class TrackFIFO;

class P1SCounter : public Counter
{
public:
    P1SCounter(const char* name);
    virtual ~P1SCounter();

private:
    void Pace(uint64_t uInc, uint32_t uLogMod) override;

private:
    std::string m_strName;
    std::chrono::steady_clock::time_point m_tpStart;
    std::chrono::steady_clock::time_point m_tpTrace;
    std::chrono::steady_clock::time_point m_tpLast;
    std::chrono::steady_clock::duration m_duMin;
    std::chrono::steady_clock::duration m_duMax;
    uint64_t m_uTotal;
    uint64_t m_uTrace;
    TrackFIFO* m_pFIFO;
};

}  // namespace tracejj
