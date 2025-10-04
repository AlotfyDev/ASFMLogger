/**
 * DatabaseLogger Unit Test Stub - Basic API verification
 */

#include <gtest/gtest.h>
#include "stateful/DatabaseLogger.hpp"

TEST(DatabaseLoggerStubTest, CanCreateLogger) {
    DatabaseLogger logger;
    EXPECT_TRUE(logger.isConnected());
}

TEST(DatabaseLoggerStubTest, BasicOperations) {
    DatabaseLogger logger;
    LogMessageData msg;
    msg.type = LogMessageType::INFO;
    strcpy(msg.message, "Test log");
    strcpy(msg.component, "TestComp");

    DatabaseOperationResult result = logger.insertMessage(msg);
    EXPECT_NE(result.operation_id, 0u);
}

TEST(DatabaseLoggerStubTest, PerformanceStub) {
    DatabaseLogger logger;

    LARGE_INTEGER start, end, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    for (size_t i = 0; i < 50; ++i) {
        LogMessageData msg;
        msg.type = LogMessageType::INFO;
        strcpy(msg.message, "Perf log");
        strcpy(msg.component, "PerfComp");

        logger.insertMessage(msg);
    }

    QueryPerformanceCounter(&end);
    double duration_ms = ((end.QuadPart - start.QuadPart) * 1000.0) / freq.QuadPart;

    EXPECT_LT(duration_ms, 10000.0);
    std::cout << "[DatabaseLogger Stub] " << 50 << " operations in " << duration_ms << "ms" << std::endl;
}
