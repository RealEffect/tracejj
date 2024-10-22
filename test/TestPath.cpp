#include <gtest/gtest.h>
#include <future>
#include "platform/Path.h"

TEST(Sytem, GetExecutablePath)
{
    ASSERT_FALSE(GetExecutablePath().empty());
}

TEST(Sytem, GetLogsPath)
{
    ASSERT_FALSE(GetDefaultLogPath().empty());
}
