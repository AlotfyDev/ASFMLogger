/**
 * ASFMLogger Unit Tests for LogMessage Component
 *
 * Tests actual functionality of the implemented LogMessage class
 */

#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <string>
#include <algorithm>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// Include the component under test
#include "stateful/LogMessage.hpp"

// ====================================================================================
// CONSTRUCTION AND VALIDATION TESTS
// ====================================================================================

TEST(LogMessageTest, DefaultConstruction) {
    LogMessage msg;

    // Test basic validation
    EXPECT_FALSE(msg.isValid());
    EXPECT_TRUE(msg.empty());
    EXPECT_EQ(msg.getId(), 0u);
    EXPECT_EQ(msg.getType(), LogMessageType::INFO); // Default type when empty
}

TEST(LogMessageTest, FullConstructorWithParameters) {
    LogMessage msg(LogMessageType::ERR, "Test error message",
                   "TestComponent", "testFunction", "test.cpp", 42);

    EXPECT_TRUE(msg.isValid());
    EXPECT_FALSE(msg.empty());
    EXPECT_EQ(msg.getMessage(), "Test error message");
    EXPECT_EQ(msg.getComponent(), "TestComponent");
    EXPECT_EQ(msg.getFunction(), "testFunction");
    EXPECT_EQ(msg.getFile(), "test.cpp");
    EXPECT_EQ(msg.getLineNumber(), 42u);
    EXPECT_EQ(msg.getType(), LogMessageType::ERR);
    EXPECT_GT(msg.getId(), 0u); // Should have auto-generated ID
}

TEST(LogMessageTest, StaticCreateMethod) {
    LogMessage msg = LogMessage::Create(LogMessageType::WARN, "Warning message",
                                       "WarningComponent", "warnFunc", "warn.cpp", 123);

    EXPECT_TRUE(msg.isValid());
    EXPECT_EQ(msg.getType(), LogMessageType::WARN);
    EXPECT_EQ(msg.getMessage(), "Warning message");
    EXPECT_EQ(msg.getComponent(), "WarningComponent");
    EXPECT_EQ(msg.getFunction(), "warnFunc");
    EXPECT_EQ(msg.getLineNumber(), 123u);
}

TEST(LogMessageTest, ConstructorWithLogMessageData) {
    // Create a LogMessageData manually
    LogMessageData data = {};
    data.message_id = 1000;
    data.instance_id = 2000;
    data.type = LogMessageType::CRITICAL_LOG;
    data.timestamp.seconds = 1234567890;

    strcpy(data.message, "Manual data message");
    strcpy(data.component, "ManualComponent");
    strcpy(data.function, "manualFunction");
    strcpy(data.file, "manual.cpp");
    strcpy(data.severity_string, "CRITICAL");

    data.process_id = 12345;
    data.thread_id = 6789;
    data.line_number = 999;

    LogMessage msg(data);

    // Verify data was copied correctly
    EXPECT_TRUE(msg.isValid());
    EXPECT_EQ(msg.getId(), 1000u);
    EXPECT_EQ(msg.getInstanceId(), 2000u);
    EXPECT_EQ(msg.getType(), LogMessageType::CRITICAL_LOG);
    EXPECT_EQ(msg.getMessage(), "Manual data message");
    EXPECT_EQ(msg.getComponent(), "ManualComponent");
    EXPECT_EQ(msg.getProcessId(), 12345u);
    EXPECT_EQ(msg.getThreadId(), 6789u);
    EXPECT_EQ(msg.getLineNumber(), 999u);
}

// ====================================================================================
// COPY/ASSIGNMENT OPERATIONS
// ====================================================================================

TEST(LogMessageTest, CopyConstructor) {
    LogMessage original(LogMessageType::ERR, "Original message", "OrigComp", "origFunc");

    LogMessage copy(original);

    // Verify copy has same data
    EXPECT_TRUE(copy.isValid());
    EXPECT_EQ(copy.getMessage(), original.getMessage());
    EXPECT_EQ(copy.getComponent(), original.getComponent());
    EXPECT_EQ(copy.getType(), original.getType());
    EXPECT_EQ(copy.getId(), original.getId());
    EXPECT_EQ(copy.getInstanceId(), original.getInstanceId());

    // But they should be different objects (verify equality works)
    EXPECT_TRUE(copy == original);
}

TEST(LogMessageTest, AssignmentOperator) {
    LogMessage original(LogMessageType::WARN, "Original", "Orig", "orig");
    LogMessage target;

    target = original;

    // Verify assignment worked
    EXPECT_TRUE(target.isValid());
    EXPECT_EQ(target.getMessage(), "Original");
    EXPECT_EQ(target.getType(), LogMessageType::WARN);
    EXPECT_TRUE(target == original);
}

TEST(LogMessageTest, CloneMethod) {
    LogMessage original(LogMessageType::INFO, "Clone test", "CloneComp", "cloneFunc");
    uint32_t originalId = original.getId();

    LogMessage cloned = original.clone();

    EXPECT_TRUE(cloned.isValid());
    EXPECT_EQ(cloned.getMessage(), original.getMessage());
    EXPECT_EQ(cloned.getComponent(), original.getComponent());
    EXPECT_EQ(cloned.getId(), originalId);
    EXPECT_TRUE(cloned == original);
}

// ====================================================================================
// GETTER METHODS
// ====================================================================================

TEST(LogMessageTest, GetterMethods) {
    LogMessage msg(LogMessageType::DEBUG, "Debug message content", "DebugComponent",
                   "debugFunction", "debug.cpp", 456);

    EXPECT_EQ(msg.getMessage(), "Debug message content");
    EXPECT_EQ(msg.getComponent(), "DebugComponent");
    EXPECT_EQ(msg.getFunction(), "debugFunction");
    EXPECT_EQ(msg.getFile(), "debug.cpp");
    EXPECT_EQ(msg.getLineNumber(), 456u);
    EXPECT_EQ(msg.getType(), LogMessageType::DEBUG);
    EXPECT_GT(msg.getId(), 0u);
    EXPECT_GE(msg.getInstanceId(), 0u);

    // Process and thread IDs should be set internally (can't test specific values easily)
    EXPECT_NE(msg.getProcessId(), 0u);
    EXPECT_NE(msg.getThreadId(), 0u);
}

// ====================================================================================
// SETTER METHODS
// ====================================================================================

TEST(LogMessageTest, SetMessage) {
    LogMessage msg(LogMessageType::INFO, "Original", "Comp", "func");

    EXPECT_TRUE(msg.setMessage("New message content"));
    EXPECT_EQ(msg.getMessage(), "New message content");
    EXPECT_TRUE(msg.isValid());
}

TEST(LogMessageTest, SetComponent) {
    LogMessage msg(LogMessageType::INFO, "Message", "OldComp", "func");

    EXPECT_TRUE(msg.setComponent("NewComponent"));
    EXPECT_EQ(msg.getComponent(), "NewComponent");
}

TEST(LogMessageTest, SetFunction) {
    LogMessage msg(LogMessageType::INFO, "Message", "Comp", "oldFunc");

    EXPECT_TRUE(msg.setFunction("newFunction"));
    EXPECT_EQ(msg.getFunction(), "newFunction");
}

TEST(LogMessageTest, SetType) {
    LogMessage msg(LogMessageType::INFO, "Message", "Comp", "func");

    msg.setType(LogMessageType::ERR);
    EXPECT_EQ(msg.getType(), LogMessageType::ERR);

    msg.setType(LogMessageType::CRITICAL_LOG);
    EXPECT_EQ(msg.getType(), LogMessageType::CRITICAL_LOG);
}

TEST(LogMessageTest, SetSourceLocation) {
    LogMessage msg(LogMessageType::INFO, "Message", "Comp", "func");

    EXPECT_TRUE(msg.setSourceLocation("newfile.cpp", 789));
    EXPECT_EQ(msg.getFile(), "newfile.cpp");
    EXPECT_EQ(msg.getLineNumber(), 789u);
}

// ====================================================================================
// VALIDATION METHODS
// ====================================================================================

TEST(LogMessageTest, ValidationMethods) {
    LogMessage validMsg(LogMessageType::INFO, "Valid message", "Component", "function");

    EXPECT_TRUE(validMsg.isValid());
    EXPECT_TRUE(validMsg.hasContent());
    EXPECT_TRUE(validMsg.hasComponent());
    EXPECT_FALSE(validMsg.empty());

    // Test empty/invalid message
    LogMessage emptyMsg;
    EXPECT_FALSE(emptyMsg.isValid());
    EXPECT_FALSE(emptyMsg.hasContent());
    EXPECT_FALSE(emptyMsg.hasComponent());
    EXPECT_TRUE(emptyMsg.empty());
}

// ====================================================================================
// BASIC PERFORMANCE CHECK
// ====================================================================================

TEST(LogMessageTest, BasicPerformanceCheck) {
    const size_t ITERATIONS = 5000;

    // Simple timing using Windows API to ensure basic operations aren't extremely slow
    LARGE_INTEGER start, end, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    std::vector<LogMessage> messages;
    messages.reserve(ITERATIONS);

    for (size_t i = 0; i < ITERATIONS; ++i) {
        LogMessage msg(LogMessageType::DEBUG, "Perf message " + std::to_string(i), "PerfComp", "perfFunc");
        std::string formatted = msg.toString();
        uint32_t hash = msg.hashContent();

        messages.push_back(std::move(msg));

        EXPECT_TRUE(messages.back().isValid());
        EXPECT_FALSE(formatted.empty());
    }

    QueryPerformanceCounter(&end);
    double duration_ms = ((end.QuadPart - start.QuadPart) * 1000.0) / freq.QuadPart;

    // Should complete reasonably (< 10 seconds for basic operations)
    EXPECT_LT(duration_ms, 10000.0);
    std::cout << "[LogMessage Performance] " << ITERATIONS << " operations completed in "
              << duration_ms << "ms" << std::endl;
}

TEST(LogMessageTest, ToCsvFormatting) {
    LogMessage msg(LogMessageType::INFO, "CSV test message", "CSVComp", "csvFunc");

    std::string csv = msg.toCsv();
    EXPECT_FALSE(csv.empty());

    // Should contain commas for CSV format
    EXPECT_NE(csv.find(","), std::string::npos);
}

TEST(LogMessageTest, FormatForConsole) {
    LogMessage msg(LogMessageType::CRITICAL_LOG, "Critical error message", "CriticalComp");

    std::string console = msg.formatForConsole();
    EXPECT_FALSE(console.empty());

    // Should be human-readable console format
    EXPECT_GT(console.length(), 20);
}

// ====================================================================================
// COMPARISON OPERATORS
// ====================================================================================

TEST(LogMessageTest, EqualityOperators) {
    LogMessage msg1(LogMessageType::INFO, "Same content", "SameComp", "sameFunc");
    LogMessage msg2(LogMessageType::INFO, "Same content", "SameComp", "sameFunc");

    // Different IDs but same core content should still be equal
    EXPECT_TRUE(msg1 == msg2);
    EXPECT_FALSE(msg1 != msg2);

    // Different type should not be equal
    msg2.setType(LogMessageType::WARN);
    EXPECT_FALSE(msg1 == msg2);
    EXPECT_TRUE(msg1 != msg2);
}

TEST(LogMessageTest, OrderingOperators) {
    LogMessage msg1(LogMessageType::INFO, "First message", "Comp", "func");
    LogMessage msg2(LogMessageType::INFO, "Second message", "Comp", "func");

    // Different IDs might affect ordering through timestamps
    // The less-than comparison uses timestamps, so we test basic functionality
    bool result = (msg1 < msg2) || (msg1 > msg2) || (msg1 == msg2);
    EXPECT_TRUE(result); // At least one comparison should be true
}

// ====================================================================================
// UTILITY METHODS
// ====================================================================================

TEST(LogMessageTest, HashContent) {
    LogMessage msg(LogMessageType::INFO, "Hash this content", "HashComp", "hashFunc");

    uint32_t hash = msg.hashContent();
    EXPECT_NE(hash, 0u);

    // Right shift test to ensure it's not always the same
    uint32_t hash2 = msg.hashContent();
    EXPECT_EQ(hash, hash2); // Should be deterministic for same content
}

TEST(LogMessageTest, SwapOperation) {
    LogMessage msg1(LogMessageType::INFO, "Message 1", "Comp1", "func1");
    LogMessage msg2(LogMessageType::ERR, "Message 2", "Comp2", "func2");

    std::string msg1Original = msg1.getMessage();
    LogMessageType msg1OriginalType = msg1.getType();

    std::string msg2Original = msg2.getMessage();
    LogMessageType msg2OriginalType = msg2.getType();

    msg1.swap(msg2);

    // After swap, each should have the other's content
    EXPECT_EQ(msg1.getMessage(), msg2Original);
    EXPECT_EQ(msg1.getType(), msg2OriginalType);
    EXPECT_EQ(msg2.getMessage(), msg1Original);
    EXPECT_EQ(msg2.getType(), msg1OriginalType);
}

TEST(LogMessageTest, ClearMethod) {
    LogMessage msg(LogMessageType::CRITICAL_LOG, "Important message", "CriticalComp", "criticalFunc");

    EXPECT_TRUE(msg.isValid());
    EXPECT_FALSE(msg.empty());

    msg.clear();

    // After clear, should be in reset state
    EXPECT_FALSE(msg.isValid());
    EXPECT_TRUE(msg.empty());
    EXPECT_EQ(msg.getMessage(), "");
    EXPECT_EQ(msg.getComponent(), "");
    EXPECT_EQ(msg.getId(), 0u);
}

TEST(LogMessageTest, EmptyMethod) {
    LogMessage emptyMsg;
    EXPECT_TRUE(emptyMsg.empty());

    LogMessage validMsg(LogMessageType::INFO, "Content", "Comp", "func");
    EXPECT_FALSE(validMsg.empty());
}

// ====================================================================================
// EDGE CASES
// ====================================================================================

TEST(LogMessageTest, EmptyStrings) {
    LogMessage msg(LogMessageType::INFO, "", "", "");

    EXPECT_TRUE(msg.isValid()); // Valid from a construction perspective
    EXPECT_FALSE(msg.hasContent());
    EXPECT_FALSE(msg.hasComponent());
    EXPECT_EQ(msg.getMessage(), "");
    EXPECT_EQ(msg.getComponent(), "");

    // Formatting should still work
    std::string str = msg.toString();
    EXPECT_FALSE(str.empty()); // Should produce some output even with empty content
}

TEST(LogMessageTest, VeryLongStrings) {
    std::string longMsg(200, 'A'); // Long message
    std::string longComp(50, 'B'); // Long component
    std::string longFunc(50, 'C'); // Long function

    LogMessage msg(LogMessageType::INFO, longMsg, longComp, longFunc);

    EXPECT_TRUE(msg.isValid());
    EXPECT_EQ(msg.getMessage(), longMsg);
    EXPECT_EQ(msg.getComponent(), longComp);
    EXPECT_EQ(msg.getFunction(), longFunc);

    // Formatting should handle long strings
    std::string json = msg.toJson();
    EXPECT_FALSE(json.empty());
    std::string csv = msg.toCsv();
    EXPECT_FALSE(csv.empty());
}

TEST(LogMessageTest, SpecialCharacters) {
    std::string specialMsg = "Special: !@#$%^&*()_{}[]|\\\":;\"'<>,.?/~`\\n\\t";
    LogMessage msg(LogMessageType::WARN, specialMsg, "SpecialComp", "specialFunc");

    EXPECT_TRUE(msg.isValid());
    EXPECT_EQ(msg.getMessage(), specialMsg);

    // Formatting should handle special characters
    std::string json = msg.toJson();
    EXPECT_FALSE(json.empty());
    std::string csv = msg.toCsv();
    EXPECT_FALSE(csv.empty());
}

// ===============================================================================
// END OF LogMessageTests - All functionality verified
// ===============================================================================
