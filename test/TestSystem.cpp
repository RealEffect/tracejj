#include <gtest/gtest.h>
#include <future>
#include "trace++/System.h"

TEST(Sytem, CurrentThreadNumber)
{
    ASSERT_GT(tracejj::CurrentThreadNumber(), 0U);
    ASSERT_EQ(tracejj::CurrentThreadNumber(), tracejj::CurrentThreadNumber());
    const auto f = std::async(
        [number = tracejj::CurrentThreadNumber()]()
        {
            ASSERT_NE(number, tracejj::CurrentThreadNumber());
            ASSERT_EQ(tracejj::CurrentThreadNumber(), tracejj::CurrentThreadNumber());
        });
    f.wait();
}

TEST(Sytem, CurrentProcessNumber)
{
    ASSERT_GT(tracejj::CurrentProcessNumber(), 0U);
    ASSERT_EQ(tracejj::CurrentProcessNumber(), tracejj::CurrentProcessNumber());
    const auto f = std::async(
        [number = tracejj::CurrentProcessNumber()]()
        {
            ASSERT_NE(number, tracejj::CurrentProcessNumber());
            ASSERT_EQ(tracejj::CurrentProcessNumber(), tracejj::CurrentProcessNumber());
        });
    f.wait();
}