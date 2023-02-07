#include <gmock/gmock.h>
#include <trace++/Logger.h>

class MockLogWriter : public LogWriter
{
public:
    MOCK_METHOD(bool, Write, (LogLevel level, const char* strMsg, size_t szMsgLength), (override));
};

class MockPrintObserver : public Logger::Observer
{
public:
    MOCK_METHOD(void, OnMessage, (LogLevel level, const char* strMsg, size_t szMsg), (override));
};

TEST(Logger, API)
{
    auto& logger = Logger::Instance();
    EXPECT_EQ(logger.MaxLevel(), LogLevel::MAX);
    logger.SetMaxLevel(LogLevel::NOTICE);
    EXPECT_EQ(logger.MaxLevel(), LogLevel::NOTICE);
    for (uint32_t i = 0U; i < 16U; i++)
    {
        EXPECT_TRUE(logger.SetWriter(i, nullptr));
    }
    EXPECT_FALSE(logger.SetWriter(16U, nullptr));
    std::unique_ptr<MockLogWriter> pWriter(new MockLogWriter);
    EXPECT_TRUE(logger.SetWriter(0U, pWriter.get()));
    EXPECT_TRUE(logger.SetWriter(1U, pWriter.get()));
    EXPECT_FALSE(logger.SetWriter(16U, pWriter.get()));
    EXPECT_TRUE(logger.SetWriter(0U, nullptr));
    EXPECT_TRUE(logger.SetWriter(1U, nullptr));
}

TEST(Logger, Print)
{
    auto& logger = Logger::Instance();
    std::unique_ptr<MockPrintObserver> pObserver(new MockPrintObserver);
    EXPECT_CALL(*pObserver, OnMessage(LogLevel::FATAL, testing::_, 1u)).Times(1);
    EXPECT_CALL(*pObserver, OnMessage(LogLevel::ALERT, testing::_, 1u)).Times(1);
    EXPECT_CALL(*pObserver, OnMessage(LogLevel::CRIT, testing::_, 1u)).Times(1);
    EXPECT_CALL(*pObserver, OnMessage(LogLevel::FAULT, testing::_, 1u)).Times(1);
    EXPECT_CALL(*pObserver, OnMessage(LogLevel::NOTICE, testing::_, 1u)).Times(1);
    EXPECT_CALL(*pObserver, OnMessage(LogLevel::INFO, testing::_, 1u)).Times(1);
    EXPECT_CALL(*pObserver, OnMessage(LogLevel::DEBUG, testing::_, 1u)).Times(1);
    EXPECT_CALL(*pObserver, OnMessage(LogLevel::MAX, testing::_, 1u)).Times(0);
    logger.SetMaxLevel(LogLevel::MAX);
    logger.SetPrintObserver(pObserver.get());
    logger.Print(LogLevel::FATAL, "1");
    logger.Print(LogLevel::ALERT, "12", 1);
    logger.Print(LogLevel::CRIT, "1");
    logger.Print(LogLevel::FAULT, "1");
    logger.Print(LogLevel::NOTICE, "1");
    logger.Print(LogLevel::INFO, "123", 1);
    logger.Print(LogLevel::DEBUG, "1");
    logger.SetMaxLevel(LogLevel::NOTICE);
    EXPECT_CALL(*pObserver, OnMessage(LogLevel::FATAL, testing::_, 1u)).Times(1);
    EXPECT_CALL(*pObserver, OnMessage(LogLevel::ALERT, testing::_, 1u)).Times(1);
    EXPECT_CALL(*pObserver, OnMessage(LogLevel::CRIT, testing::_, 1u)).Times(1);
    EXPECT_CALL(*pObserver, OnMessage(LogLevel::FAULT, testing::_, 1u)).Times(1);
    EXPECT_CALL(*pObserver, OnMessage(LogLevel::NOTICE, testing::_, 1u)).Times(1);
    EXPECT_CALL(*pObserver, OnMessage(LogLevel::INFO, testing::_, 1u)).Times(0);
    EXPECT_CALL(*pObserver, OnMessage(LogLevel::DEBUG, testing::_, 1u)).Times(0);
    EXPECT_CALL(*pObserver, OnMessage(LogLevel::MAX, testing::_, 1u)).Times(0);
    logger.Print(LogLevel::FATAL, "1");
    logger.Print(LogLevel::ALERT, "1");
    logger.Print(LogLevel::CRIT, "1");
    logger.Print(LogLevel::FAULT, "1");
    logger.Print(LogLevel::NOTICE, "1");
    logger.Print(LogLevel::INFO, "1");
    logger.Print(LogLevel::DEBUG, "1");
}

TEST(Logger, Write)
{
    auto& logger = Logger::Instance();
    std::unique_ptr<MockLogWriter> pWriter(new MockLogWriter);
    EXPECT_CALL(*pWriter, Write(LogLevel::FATAL, testing::_, 1u)).Times(1);
    EXPECT_CALL(*pWriter, Write(LogLevel::ALERT, testing::_, 1u)).Times(1);
    EXPECT_CALL(*pWriter, Write(LogLevel::CRIT, testing::_, 1u)).Times(1);
    EXPECT_CALL(*pWriter, Write(LogLevel::FAULT, testing::_, 1u)).Times(1);
    EXPECT_CALL(*pWriter, Write(LogLevel::NOTICE, testing::_, 1u)).Times(1);
    EXPECT_CALL(*pWriter, Write(LogLevel::INFO, testing::_, 1u)).Times(1);
    EXPECT_CALL(*pWriter, Write(LogLevel::DEBUG, testing::_, 1u)).Times(1);
    EXPECT_CALL(*pWriter, Write(LogLevel::MAX, testing::_, 1u)).Times(0);
    logger.SetMaxLevel(LogLevel::MAX);
    logger.SetWriter(0U, pWriter.get());
    logger.Write(LogLevel::FATAL, 0U, "1");
    logger.Write(LogLevel::ALERT, 0U, "12", 1);
    logger.Write(LogLevel::CRIT, 1U, "1");
    logger.Write(LogLevel::FAULT, 2u, "1");
    logger.Write(LogLevel::NOTICE, 0U, "123", 1);
    logger.Write(LogLevel::INFO, 0U, "1");
    logger.Write(LogLevel::DEBUG, 0U, "1");

    logger.SetMaxLevel(LogLevel::NOTICE);
    EXPECT_CALL(*pWriter, Write(LogLevel::FATAL, testing::_, 1u)).Times(1);
    EXPECT_CALL(*pWriter, Write(LogLevel::ALERT, testing::_, 1u)).Times(1);
    EXPECT_CALL(*pWriter, Write(LogLevel::CRIT, testing::_, 1u)).Times(0);
    EXPECT_CALL(*pWriter, Write(LogLevel::FAULT, testing::_, 1u)).Times(0);
    EXPECT_CALL(*pWriter, Write(LogLevel::NOTICE, testing::_, 1u)).Times(1);
    EXPECT_CALL(*pWriter, Write(LogLevel::INFO, testing::_, 1u)).Times(0);
    EXPECT_CALL(*pWriter, Write(LogLevel::DEBUG, testing::_, 1u)).Times(0);
    EXPECT_CALL(*pWriter, Write(LogLevel::MAX, testing::_, 1u)).Times(0);
    std::unique_ptr<MockLogWriter> pWriter1(new MockLogWriter);
    EXPECT_CALL(*pWriter1, Write(LogLevel::CRIT, testing::_, 1u)).Times(1);
    logger.SetWriter(1U, pWriter1.get());
    std::unique_ptr<MockLogWriter> pWriter2(new MockLogWriter);
    EXPECT_CALL(*pWriter2, Write(LogLevel::FAULT, testing::_, 1u)).Times(1);
    logger.SetWriter(2U, pWriter2.get());
    logger.Write(LogLevel::FATAL, 0U, "1");
    logger.Write(LogLevel::ALERT, 0U, "12", 1);
    logger.Write(LogLevel::CRIT, 1U, "1");
    logger.Write(LogLevel::FAULT, 2u, "1");
    logger.Write(LogLevel::NOTICE, 0U, "123", 1);
    logger.Write(LogLevel::INFO, 0U, "1");
    logger.Write(LogLevel::DEBUG, 0U, "1");
}