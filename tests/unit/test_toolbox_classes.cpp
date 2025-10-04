/**
 * ASFMLogger Toolbox Classes Test Suite
 *
 * Tests for all static toolbox classes following the 3-layer architecture
 */

#include <gtest/gtest.h>
#include <thread>
#include <atomic>
#include <vector>
#include <string>

// Include toolbox headers
#include "../src/structs/LogDataStructures.hpp"
#include "../src/toolbox/LogMessageToolbox.hpp"
#include "../src/toolbox/TimestampToolbox.hpp"
#include "../src/toolbox/LoggerInstanceToolbox.hpp"
#include "../src/toolbox/ImportanceToolbox.hpp"

class ToolboxTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize toolbox systems
        ImportanceToolbox::InitializeDefaultMapping();
    }

    void TearDown() override {
        // Cleanup
    }
};

// Test LogMessageToolbox
TEST_F(ToolboxTest, LogMessageToolbox_IDGeneration) {
    uint32_t id1 = LogMessageToolbox::GenerateMessageId();
    uint32_t id2 = LogMessageToolbox::GenerateMessageId();

    EXPECT_NE(id1, id2);
    EXPECT_LT(id1, id2);  // Should be sequential
}

TEST_F(ToolboxTest, LogMessageToolbox_MessageCreation) {
    LogMessageData data = LogMessageToolbox::CreateMessage(
        LogMessageType::INFO,
        "Test message",
        "TestComponent",
        "TestFunction"
    );

    EXPECT_EQ(data.type, LogMessageType::INFO);
    EXPECT_STREQ(data.message, "Test message");
    EXPECT_STREQ(data.component, "TestComponent");
    EXPECT_STREQ(data.function, "TestFunction");
    EXPECT_NE(data.message_id, 0);
    EXPECT_NE(data.timestamp.seconds, 0);
}

TEST_F(ToolboxTest, LogMessageToolbox_MessageValidation) {
    LogMessageData valid_data = LogMessageToolbox::CreateMessage(
        LogMessageType::ERROR, "Valid message"
    );

    EXPECT_TRUE(LogMessageToolbox::ValidateMessage(valid_data));

    LogMessageData invalid_data;
    memset(&invalid_data, 0, sizeof(LogMessageData));
    EXPECT_FALSE(LogMessageToolbox::ValidateMessage(invalid_data));
}

TEST_F(ToolboxTest, LogMessageToolbox_MessageFormatting) {
    LogMessageData data = LogMessageToolbox::CreateMessage(
        LogMessageType::WARN, "Test warning message"
    );

    std::string formatted = LogMessageToolbox::MessageToString(data);
    EXPECT_FALSE(formatted.empty());
    EXPECT_NE(formatted.find("Test warning message"), std::string::npos);
}

// Test TimestampToolbox
TEST_F(ToolboxTest, TimestampToolbox_NowReturnsIncreasingTime) {
    auto ts1 = TimestampToolbox::Now();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    auto ts2 = TimestampToolbox::Now();

    EXPECT_LT(ts1.seconds, ts2.seconds);
}

TEST_F(ToolboxTest, TimestampToolbox_FormatTimestamp) {
    LogTimestamp ts;
    ts.seconds = 1640995200;  // 2022-01-01 00:00:00 UTC
    ts.microseconds = 500000;

    std::string formatted = TimestampToolbox::Format(ts);
    EXPECT_FALSE(formatted.empty());
}

// Test LoggerInstanceToolbox
TEST_F(ToolboxTest, LoggerInstanceToolbox_InstanceCreation) {
    LoggerInstanceData instance = LoggerInstanceToolbox::CreateInstance(
        "TestApp", "TestProcess", "TestInstance"
    );

    EXPECT_STREQ(instance.application_name, "TestApp");
    EXPECT_STREQ(instance.process_name, "TestProcess");
    EXPECT_STREQ(instance.instance_name, "TestInstance");
    EXPECT_NE(instance.instance_id, 0);
    EXPECT_NE(instance.creation_time, 0);
}

TEST_F(ToolboxTest, LoggerInstanceToolbox_ActivityTracking) {
    LoggerInstanceData instance = LoggerInstanceToolbox::CreateInstance("TestApp");

    DWORD initial_activity = instance.last_activity;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    LoggerInstanceToolbox::UpdateActivity(instance);
    EXPECT_GT(instance.last_activity, initial_activity);
}

TEST_F(ToolboxTest, LoggerInstanceToolbox_Statistics) {
    LoggerInstanceData instance = LoggerInstanceToolbox::CreateInstance("TestApp");

    // Update statistics
    LoggerInstanceToolbox::UpdateStatistics(instance, 100, 5);

    EXPECT_EQ(instance.message_count, 100);
    EXPECT_EQ(instance.error_count, 5);
}

// Test ImportanceToolbox
TEST_F(ToolboxTest, ImportanceToolbox_DefaultMapping) {
    ImportanceToolbox::InitializeDefaultMapping();

    EXPECT_EQ(ImportanceToolbox::GetDefaultImportance(LogMessageType::TRACE),
              MessageImportance::LOW);
    EXPECT_EQ(ImportanceToolbox::GetDefaultImportance(LogMessageType::ERROR),
              MessageImportance::CRITICAL);
}

TEST_F(ToolboxTest, ImportanceToolbox_TypeImportance) {
    auto importance = ImportanceToolbox::ResolveTypeImportance(LogMessageType::INFO);
    EXPECT_GE(importance, MessageImportance::LOW);
    EXPECT_LE(importance, MessageImportance::CRITICAL);
}

TEST_F(ToolboxTest, ImportanceToolbox_ComponentImportance) {
    auto importance = ImportanceToolbox::ResolveComponentImportance(
        LogMessageType::ERROR, "Database"
    );
    EXPECT_GE(importance, MessageImportance::HIGH);  // Database errors should be high importance
}

// Test thread safety
TEST_F(ToolboxTest, ToolboxThreadSafety) {
    const int NUM_THREADS = 10;
    const int OPERATIONS_PER_THREAD = 1000;

    std::atomic<int> success_count{0};
    std::vector<std::thread> threads;

    // Test concurrent ID generation
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&success_count]() {
            for (int j = 0; j < OPERATIONS_PER_THREAD; ++j) {
                uint32_t id = LogMessageToolbox::GenerateMessageId();
                if (id != 0) {
                    success_count++;
                }
            }
        });
    }

    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(success_count, NUM_THREADS * OPERATIONS_PER_THREAD);
}

// Test performance
TEST_F(ToolboxTest, ToolboxPerformance) {
    const int NUM_OPERATIONS = 10000;

    // Test message creation performance
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        LogMessageData data = LogMessageToolbox::CreateMessage(
            LogMessageType::INFO, "Performance test message"
        );
        EXPECT_TRUE(LogMessageToolbox::ValidateMessage(data));
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start).count();

    // Should be able to create 10k messages in under 100ms
    EXPECT_LT(duration, 100.0);
    std::cout << "Created " << NUM_OPERATIONS << " messages in " << duration << "ms" << std::endl;
}

// Test memory safety
TEST_F(ToolboxTest, ToolboxMemorySafety) {
    const int NUM_ITERATIONS = 1000;

    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        // Test string operations don't cause buffer overflows
        std::string long_message(2000, 'A');  // Very long message

        LogMessageData data = LogMessageToolbox::CreateMessage(
            LogMessageType::INFO, long_message
        );

        // Should handle long messages gracefully
        EXPECT_TRUE(LogMessageToolbox::ValidateMessage(data));
        EXPECT_LT(strlen(data.message), sizeof(data.message));
    }
}

// Test batch operations
TEST_F(ToolboxTest, ToolboxBatchOperations) {
    std::vector<LogMessageData> messages;

    // Create batch of messages
    for (int i = 0; i < 100; ++i) {
        messages.push_back(LogMessageToolbox::CreateMessage(
            static_cast<LogMessageType>(i % 6),  // All message types
            "Batch message " + std::to_string(i)
        ));
    }

    // Test batch validation
    auto validation_results = LogMessageToolbox::ValidateMessages(messages);
    EXPECT_EQ(validation_results.size(), messages.size());

    for (bool result : validation_results) {
        EXPECT_TRUE(result);
    }

    // Test filtering by type
    auto error_messages = LogMessageToolbox::FilterByType(messages, LogMessageType::ERR);
    EXPECT_GT(error_messages.size(), 0);

    for (const auto& msg : error_messages) {
        EXPECT_EQ(msg.type, LogMessageType::ERR);
    }
}

// Test error handling
TEST_F(ToolboxTest, ToolboxErrorHandling) {
    // Test with empty/null data
    LogMessageData empty_data;
    memset(&empty_data, 0, sizeof(LogMessageData));

    EXPECT_FALSE(LogMessageToolbox::ValidateMessage(empty_data));
    EXPECT_FALSE(LogMessageToolbox::HasContent(empty_data));

    // Test with invalid parameters
    LogMessageData data;
    EXPECT_FALSE(LogMessageToolbox::SetMessage(data, ""));  // Empty message should fail
    EXPECT_FALSE(LogMessageToolbox::SetComponent(data, ""));  // Empty component should fail
}

// Test string operations
TEST_F(ToolboxTest, ToolboxStringOperations) {
    LogMessageData data;

    // Test string field operations
    EXPECT_TRUE(LogMessageToolbox::SetMessage(data, "Test message"));
    EXPECT_STREQ(LogMessageToolbox::ExtractMessage(data).c_str(), "Test message");

    EXPECT_TRUE(LogMessageToolbox::SetComponent(data, "TestComponent"));
    EXPECT_STREQ(LogMessageToolbox::ExtractComponent(data).c_str(), "TestComponent");

    EXPECT_TRUE(LogMessageToolbox::SetFunction(data, "TestFunction"));
    EXPECT_STREQ(LogMessageToolbox::ExtractFunction(data).c_str(), "TestFunction");
}

// Test formatting functions
TEST_F(ToolboxTest, ToolboxFormatting) {
    LogMessageData data = LogMessageToolbox::CreateMessage(
        LogMessageType::ERROR, "Test error message", "TestComponent", "TestFunction"
    );

    // Test different formatting options
    std::string string_format = LogMessageToolbox::MessageToString(data);
    EXPECT_FALSE(string_format.empty());

    std::string json_format = LogMessageToolbox::MessageToJson(data);
    EXPECT_FALSE(json_format.empty());
    EXPECT_NE(json_format.find("Test error message"), std::string::npos);

    std::string csv_format = LogMessageToolbox::MessageToCsv(data);
    EXPECT_FALSE(csv_format.empty());
}

// Test collection operations
TEST_F(ToolboxTest, ToolboxCollectionOperations) {
    std::vector<LogMessageData> messages;

    // Create messages with different components
    for (int i = 0; i < 10; ++i) {
        messages.push_back(LogMessageToolbox::CreateMessage(
            LogMessageType::INFO, "Message " + std::to_string(i), "Component" + std::to_string(i % 3)
        ));
    }

    // Test filtering by component
    auto component_messages = LogMessageToolbox::FilterByComponent(messages, "Component0");
    EXPECT_GT(component_messages.size(), 0);

    for (const auto& msg : component_messages) {
        EXPECT_STREQ(msg.component, "Component0");
    }

    // Test counting by type
    auto type_counts = LogMessageToolbox::CountByType(messages);
    EXPECT_EQ(type_counts.size(), 1);  // All are INFO type
    EXPECT_EQ(type_counts[0].first, LogMessageType::INFO);
    EXPECT_EQ(type_counts[0].second, messages.size());
}

// Test sorting operations
TEST_F(ToolboxTest, ToolboxSorting) {
    std::vector<LogMessageData> messages;

    // Create messages with small delays to ensure different timestamps
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        messages.push_back(LogMessageToolbox::CreateMessage(
            LogMessageType::INFO, "Message " + std::to_string(i)
        ));
    }

    // Test sorting by timestamp
    LogMessageToolbox::SortByTimestamp(messages);

    // Verify sorting (should be in chronological order)
    for (size_t i = 1; i < messages.size(); ++i) {
        EXPECT_LE(messages[i-1].timestamp.seconds, messages[i].timestamp.seconds);
    }
}

// Test deduplication
TEST_F(ToolboxTest, ToolboxDeduplication) {
    std::vector<LogMessageData> messages;

    // Create some duplicate messages
    for (int i = 0; i < 3; ++i) {
        messages.push_back(LogMessageToolbox::CreateMessage(
            LogMessageType::INFO, "Duplicate message"
        ));
    }

    // Add a unique message
    messages.push_back(LogMessageToolbox::CreateMessage(
        LogMessageType::INFO, "Unique message"
    ));

    size_t initial_size = messages.size();
    size_t duplicates_removed = LogMessageToolbox::RemoveDuplicates(messages);

    EXPECT_EQ(duplicates_removed, 2);  // Should remove 2 duplicates
    EXPECT_EQ(messages.size(), initial_size - 2);
}

// Test hash operations
TEST_F(ToolboxTest, ToolboxHashing) {
    LogMessageData data1 = LogMessageToolbox::CreateMessage(
        LogMessageType::INFO, "Test message"
    );

    LogMessageData data2 = LogMessageToolbox::CreateMessage(
        LogMessageType::INFO, "Test message"
    );

    LogMessageData data3 = LogMessageToolbox::CreateMessage(
        LogMessageType::INFO, "Different message"
    );

    uint32_t hash1 = LogMessageToolbox::HashMessageContent(data1);
    uint32_t hash2 = LogMessageToolbox::HashMessageContent(data2);
    uint32_t hash3 = LogMessageToolbox::HashMessageContent(data3);

    EXPECT_EQ(hash1, hash2);  // Same content should have same hash
    EXPECT_NE(hash1, hash3);  // Different content should have different hash
}

// Test comparison operations
TEST_F(ToolboxTest, ToolboxComparison) {
    LogMessageData data1 = LogMessageToolbox::CreateMessage(
        LogMessageType::INFO, "Test message"
    );

    LogMessageData data2 = LogMessageToolbox::CreateMessage(
        LogMessageType::INFO, "Test message"
    );

    LogMessageData data3 = LogMessageToolbox::CreateMessage(
        LogMessageType::ERROR, "Test message"
    );

    EXPECT_TRUE(LogMessageToolbox::CompareMessages(data1, data2));
    EXPECT_FALSE(LogMessageToolbox::CompareMessages(data1, data3));
    EXPECT_TRUE(LogMessageToolbox::CompareMessageContent(data1, data2));
    EXPECT_FALSE(LogMessageToolbox::CompareMessageContent(data1, data3));
}

// Test system information gathering
TEST_F(ToolboxTest, ToolboxSystemInfo) {
    // Test current process info
    auto process_info = LoggerInstanceToolbox::GetCurrentProcessInfo();
    EXPECT_GT(process_info.first, 0);  // Should have valid PID
    EXPECT_FALSE(process_info.second.empty());  // Should have process name

    // Test current user name
    std::string user_name = LoggerInstanceToolbox::GetCurrentUserName();
    EXPECT_FALSE(user_name.empty());

    // Test hostname
    std::string hostname = LoggerInstanceToolbox::GetHostName();
    EXPECT_FALSE(hostname.empty());

    // Test current timestamp
    DWORD timestamp = LoggerInstanceToolbox::GetCurrentTimestamp();
    EXPECT_GT(timestamp, 1640995200);  // Should be after 2022
}

// Test instance name generation
TEST_F(ToolboxTest, ToolboxInstanceNameGeneration) {
    std::string name1 = LoggerInstanceToolbox::GenerateInstanceName("TestApp", "TestProcess");
    std::string name2 = LoggerInstanceToolbox::GenerateInstanceName("TestApp", "TestProcess");

    EXPECT_FALSE(name1.empty());
    EXPECT_FALSE(name2.empty());
    EXPECT_NE(name1, name2);  // Should be unique
}

// Test edge cases
TEST_F(ToolboxTest, ToolboxEdgeCases) {
    // Test with very long strings
    std::string long_string(2000, 'X');

    LogMessageData data;
    EXPECT_TRUE(LogMessageToolbox::SetMessage(data, long_string));
    EXPECT_LT(strlen(data.message), sizeof(data.message));  // Should be truncated

    // Test with special characters
    std::string special_chars = "Test\n\t\r\0message";
    EXPECT_TRUE(LogMessageToolbox::SetMessage(data, special_chars));

    // Test with empty strings
    EXPECT_FALSE(LogMessageToolbox::SetMessage(data, ""));
    EXPECT_FALSE(LogMessageToolbox::SetComponent(data, ""));
}

// Test memory usage
TEST_F(ToolboxTest, ToolboxMemoryUsage) {
    const size_t NUM_MESSAGES = 1000;

    std::vector<LogMessageData> messages;
    messages.reserve(NUM_MESSAGES);

    // Create many messages and check memory usage
    for (size_t i = 0; i < NUM_MESSAGES; ++i) {
        messages.push_back(LogMessageToolbox::CreateMessage(
            LogMessageType::INFO, "Memory test message " + std::to_string(i)
        ));
    }

    EXPECT_EQ(messages.size(), NUM_MESSAGES);

    // All messages should be valid
    for (const auto& msg : messages) {
        EXPECT_TRUE(LogMessageToolbox::ValidateMessage(msg));
    }

    // Test that we don't have memory leaks by creating and destroying many times
    for (int iteration = 0; iteration < 10; ++iteration) {
        std::vector<LogMessageData> temp_messages;
        for (int i = 0; i < 100; ++i) {
            temp_messages.push_back(LogMessageToolbox::CreateMessage(
                LogMessageType::DEBUG, "Temp message " + std::to_string(i)
            ));
        }
        // temp_messages goes out of scope here
    }
}

// Test concurrent access
TEST_F(ToolboxTest, ToolboxConcurrentAccess) {
    const int NUM_THREADS = 5;
    const int MESSAGES_PER_THREAD = 200;

    std::atomic<int> total_messages_created{0};
    std::vector<std::thread> threads;

    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < MESSAGES_PER_THREAD; ++i) {
                LogMessageData data = LogMessageToolbox::CreateMessage(
                    static_cast<LogMessageType>(i % 6),
                    "Thread " + std::to_string(t) + " message " + std::to_string(i)
                );

                EXPECT_TRUE(LogMessageToolbox::ValidateMessage(data));
                total_messages_created++;
            }
        });
    }

    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(total_messages_created, NUM_THREADS * MESSAGES_PER_THREAD);
}

// Test error conditions
TEST_F(ToolboxTest, ToolboxErrorConditions) {
    LogMessageData data;

    // Test setting invalid data
    EXPECT_FALSE(LogMessageToolbox::SetMessage(data, std::string(2000, 'X')));  // Too long

    // Test with null/empty data
    LogMessageData null_data;
    memset(&null_data, 0, sizeof(LogMessageData));

    EXPECT_FALSE(LogMessageToolbox::ValidateMessage(null_data));
    EXPECT_FALSE(LogMessageToolbox::HasContent(null_data));
    EXPECT_FALSE(LogMessageToolbox::HasComponent(null_data));
}

// Test data integrity
TEST_F(ToolboxTest, ToolboxDataIntegrity) {
    // Create message and modify it through toolbox
    LogMessageData original = LogMessageToolbox::CreateMessage(
        LogMessageType::INFO, "Original message", "OriginalComponent", "OriginalFunction"
    );

    // Modify through toolbox functions
    LogMessageData modified = original;
    EXPECT_TRUE(LogMessageToolbox::SetMessage(modified, "Modified message"));
    EXPECT_TRUE(LogMessageToolbox::SetComponent(modified, "ModifiedComponent"));
    EXPECT_TRUE(LogMessageToolbox::SetFunction(modified, "ModifiedFunction"));

    // Verify modifications
    EXPECT_STREQ(LogMessageToolbox::ExtractMessage(modified).c_str(), "Modified message");
    EXPECT_STREQ(LogMessageToolbox::ExtractComponent(modified).c_str(), "ModifiedComponent");
    EXPECT_STREQ(LogMessageToolbox::ExtractFunction(modified).c_str(), "ModifiedFunction");

    // Original should be unchanged
    EXPECT_STREQ(LogMessageToolbox::ExtractMessage(original).c_str(), "Original message");
    EXPECT_STREQ(LogMessageToolbox::ExtractComponent(original).c_str(), "OriginalComponent");
    EXPECT_STREQ(LogMessageToolbox::ExtractFunction(original).c_str(), "OriginalFunction");
}

// Test boundary conditions
TEST_F(ToolboxTest, ToolboxBoundaryConditions) {
    // Test with maximum string lengths
    std::string max_message(sizeof(((LogMessageData*)nullptr)->message) - 1, 'M');
    std::string max_component(sizeof(((LogMessageData*)nullptr)->component) - 1, 'C');

    LogMessageData data;
    EXPECT_TRUE(LogMessageToolbox::SetMessage(data, max_message));
    EXPECT_TRUE(LogMessageToolbox::SetComponent(data, max_component));

    // Test with exactly boundary sizes
    EXPECT_EQ(strlen(data.message), max_message.length());
    EXPECT_EQ(strlen(data.component), max_component.length());

    // Test with one character over boundary (should fail or truncate)
    std::string over_max = std::string(max_message.length() + 10, 'X');
    EXPECT_FALSE(LogMessageToolbox::SetMessage(data, over_max));
}

// Test toolbox isolation (no shared state issues)
TEST_F(ToolboxTest, ToolboxIsolation) {
    // Create multiple independent message sets
    std::vector<LogMessageData> set1, set2;

    for (int i = 0; i < 100; ++i) {
        set1.push_back(LogMessageToolbox::CreateMessage(
            LogMessageType::INFO, "Set1 message " + std::to_string(i)
        ));

        set2.push_back(LogMessageToolbox::CreateMessage(
            LogMessageType::DEBUG, "Set2 message " + std::to_string(i)
        ));
    }

    // Each set should maintain its own characteristics
    auto set1_info = LogMessageToolbox::FilterByType(set1, LogMessageType::INFO);
    auto set2_debug = LogMessageToolbox::FilterByType(set2, LogMessageType::DEBUG);

    EXPECT_EQ(set1_info.size(), set1.size());
    EXPECT_EQ(set2_debug.size(), set2.size());

    // No cross-contamination
    auto set1_debug = LogMessageToolbox::FilterByType(set1, LogMessageType::DEBUG);
    EXPECT_EQ(set1_debug.size(), 0);
}

// Test resource cleanup
TEST_F(ToolboxTest, ToolboxResourceCleanup) {
    // Test that toolbox operations don't leak resources
    for (int iteration = 0; iteration < 100; ++iteration) {
        std::vector<LogMessageData> messages;

        // Create and use many messages
        for (int i = 0; i < 100; ++i) {
            messages.push_back(LogMessageToolbox::CreateMessage(
                static_cast<LogMessageType>(i % 6),
                "Cleanup test message " + std::to_string(i)
            ));
        }

        // Perform operations
        LogMessageToolbox::SortByTimestamp(messages);
        auto filtered = LogMessageToolbox::FilterByType(messages, LogMessageType::ERROR);
        auto counts = LogMessageToolbox::CountByType(messages);

        // All operations should succeed
        EXPECT_FALSE(messages.empty());
        EXPECT_GE(filtered.size(), 0);
        EXPECT_FALSE(counts.empty());

        // Messages vector goes out of scope - should cleanup automatically
    }
}

// Test toolbox consistency
TEST_F(ToolboxTest, ToolboxConsistency) {
    // Test that repeated operations give consistent results
    LogMessageData data = LogMessageToolbox::CreateMessage(
        LogMessageType::WARN, "Consistency test"
    );

    // Multiple calls should give same results for same input
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(LogMessageToolbox::GetType(data), LogMessageType::WARN);
        EXPECT_EQ(LogMessageToolbox::GetId(data), data.message_id);
        EXPECT_TRUE(LogMessageToolbox::ValidateMessage(data));
        EXPECT_TRUE(LogMessageToolbox::HasContent(data));
    }

    // Hash should be consistent
    uint32_t hash1 = LogMessageToolbox::HashMessageContent(data);
    uint32_t hash2 = LogMessageToolbox::HashMessageContent(data);
    EXPECT_EQ(hash1, hash2);
}

// Test toolbox extensibility
TEST_F(ToolboxTest, ToolboxExtensibility) {
    // Test that toolbox can handle new message types and components
    std::vector<LogMessageType> all_types = {
        LogMessageType::TRACE, LogMessageType::DEBUG, LogMessageType::INFO,
        LogMessageType::WARN, LogMessageType::ERR, LogMessageType::CRITICAL_LOG
    };

    for (LogMessageType type : all_types) {
        LogMessageData data = LogMessageToolbox::CreateMessage(
            type, "Extensibility test for " + std::to_string((int)type)
        );

        EXPECT_EQ(LogMessageToolbox::GetType(data), type);
        EXPECT_TRUE(LogMessageToolbox::ValidateMessage(data));

        std::string formatted = LogMessageToolbox::MessageToString(data);
        EXPECT_FALSE(formatted.empty());
    }
}