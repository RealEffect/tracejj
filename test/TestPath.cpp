#include <gtest/gtest.h>
#include <future>
#include "system/Path.h"

TEST(Sytem, GetExecutablePath)
{
    ASSERT_FALSE(GetExecutablePath().empty());
}

TEST(Sytem, GetLogsPath)
{
    ASSERT_FALSE(GetLogsPath().empty());
}
