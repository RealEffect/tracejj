#include <gtest/gtest.h>
#include <future>
#include "trace++/System.h"

TEST(Sytem, CurrentThreadNumber)
{
    ASSERT_GT(tracepp::CurrentThreadNumber(), 0U);
    ASSERT_EQ(tracepp::CurrentThreadNumber(), tracepp::CurrentThreadNumber());
    const auto f = std::async(
        [number = tracepp::CurrentThreadNumber()]()
        {
            ASSERT_NE(number, tracepp::CurrentThreadNumber());
            ASSERT_EQ(tracepp::CurrentThreadNumber(), tracepp::CurrentThreadNumber());
        });
    f.wait();
}

TEST(Sytem, CurrentProcessNumber)
{
    ASSERT_GT(tracepp::CurrentProcessNumber(), 0U);
    ASSERT_EQ(tracepp::CurrentProcessNumber(), tracepp::CurrentProcessNumber());
    const auto f = std::async(
        [number = tracepp::CurrentProcessNumber()]()
        {
            ASSERT_NE(number, tracepp::CurrentProcessNumber());
            ASSERT_EQ(tracepp::CurrentProcessNumber(), tracepp::CurrentProcessNumber());
        });
    f.wait();
}