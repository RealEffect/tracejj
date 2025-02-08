#pragma once

#include <cstdint>

namespace tracejj
{
uint32_t CurrentThreadNumber();
int CurrentProcessNumber();

void SetThreadAlias(const char* name);
}  // namespace tracejj
