#include <gtest/gtest.h>
#include <future>
#include "trace++/System.h"

using namespace tracejj;

TEST(Sytem, CurrentThreadNumber)
{
    ASSERT_GT(CurrentThreadNumber(), 0U);
    ASSERT_EQ(CurrentThreadNumber(), CurrentThreadNumber());
    const auto f = std::async(
        [number = CurrentThreadNumber()]()
        {
            ASSERT_NE(number, CurrentThreadNumber());
            ASSERT_EQ(CurrentThreadNumber(), CurrentThreadNumber());
        });
    f.wait();
}

TEST(Sytem, CurrentProcessNumber)
{
    ASSERT_GT(CurrentProcessNumber(), 0U);
    ASSERT_EQ(CurrentProcessNumber(), CurrentProcessNumber());
    const auto f = std::async(
        [number = CurrentProcessNumber()]()
        {
            ASSERT_EQ(number, CurrentProcessNumber());
            ASSERT_EQ(CurrentProcessNumber(), CurrentProcessNumber());
        });
    f.wait();
}