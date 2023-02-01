#include <gtest/gtest.h>
#include <future>
#include <string_view>
#include <fstream>
#include "log/LogFileWriter.h"
#include "system/Path.h"

TEST(Log, LogFileWriter)
{
    auto pathLog = GetLogsPath();
    pathLog.append("log");
    std::error_code err;
    for (auto& entry : fs::directory_iterator(pathLog, err))
    {
        if (entry.is_regular_file(err) && entry.path().extension().compare(".log") == 0)
        {
            fs::remove(entry.path(), err);
        }
    }

    LogFileWriter writer;
    constexpr std::string_view strMessage1("This is test message\r\n");
    ASSERT_FALSE(writer.Write(LogLevel::WARN, nullptr, 0));
    ASSERT_FALSE(writer.Write(LogLevel::WARN, nullptr, strMessage1.size()));
    ASSERT_FALSE(writer.Write(LogLevel::WARN, strMessage1.data(), 0));
    for (int i = 0; i < 1024; ++i)
    {
        ASSERT_TRUE(writer.Write(LogLevel::WARN, strMessage1.data(), strMessage1.size()));
    }
    writer.Close();
    for (auto& entry : fs::directory_iterator(pathLog, err))
    {
        if (entry.is_regular_file(err) && entry.path().extension().compare(".log") == 0)
        {
            std::ifstream ifs(entry.path());
            std::string strLine;
            int nLine = 0;
            while (true)
            {
                std::getline(ifs, strLine);
                if (ifs.good())
                {
                    EXPECT_EQ(std::strncmp(strLine.c_str(), strMessage1.data(), strLine.length()), 0);
                    ++nLine;
                }
                else
                {
                    break;
                }
            }
            ifs.close();
            fs::remove(entry.path(), err);
            ASSERT_EQ(nLine, 1024);
        }
    }
}