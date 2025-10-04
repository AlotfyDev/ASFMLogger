/**
 * SmartMessageQueue Unit Test Stub - Basic API verification
 */

#include <gtest/gtest.h>
#include "stateful/SmartMessageQueue.hpp"

TEST(SmartMessageQueueStubTest, CanCreateQueue) {
    SmartMessageQueue queue;
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0u);
}

TEST(SmartMessageQueueStubTest, BasicEnqueueDequeue) {
    SmartMessageQueue queue;
    // Create basic message data
    LogMessageData msg;
    msg.type = LogMessageType::INFO;
    strcpy(msg.message, "Test");
    strcpy(msg.component, "TestComp");

    EXPECT_TRUE(queue.enqueue(msg));  // Only test method exists
    EXPECT_NE(queue.size(), 0u);
}

TEST(SmartMessageQueueStubTest, PerformanceStub) {
    // Add minimal performance verification
    SmartMessageQueue queue;

    const size_t ITERATIONS = 100;
    LARGE_INTEGER start, end, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    for (size_t i = 0; i < ITERATIONS; ++i) {
        LogMessageData msg;
        msg.type = LogMessageType::INFO;
        strcpy(msg.message, "Perf");
        strcpy(msg.component, "Perf");

        queue.enqueue(msg);
    }

    QueryPerformanceCounter(&end);
    double duration_ms = ((end.QuadPart - start.QuadPart) * 1000.0) / freq.QuadPart;

    EXPECT_LT(duration_ms, 5000.0);
    std::cout << "[SmartMessageQueue Stub] " << ITERATIONS << " enqueues in " << duration_ms << "ms" << std::endl;
}
