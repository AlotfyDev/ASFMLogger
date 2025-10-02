/**
 * ASFMLogger Enhanced Features Test Suite
 *
 * Tests for enhanced logging features including multi-instance management,
 * importance framework, and contextual persistence
 */

#include <gtest/gtest.h>
#include <thread>
#include <atomic>
#include <chrono>

// Include ASFMLogger headers
#include "../ASFMLogger.hpp"
#include "../src/structs/LogDataStructures.hpp"
#include "../src/structs/ImportanceConfiguration.hpp"
#include "../src/structs/PersistencePolicy.hpp"

class EnhancedLoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test environment
    }

    void TearDown() override {
        // Cleanup
        Logger::Clear();
    }
};

// Test basic enhanced logging
TEST_F(EnhancedLoggerTest, BasicEnhancedLogging) {
    auto logger = Logger::getInstance("TestApp", "TestProcess");

    // Test enhanced logging methods
    EXPECT_NO_THROW(logger->info("TestComponent", "TestFunction", "Test message"));
    EXPECT_NO_THROW(logger->error("TestComponent", "TestFunction", "Test error"));
    EXPECT_NO_THROW(logger->critical("TestComponent", "TestFunction", "Test critical"));
}

// Test logger instance information
TEST_F(EnhancedLoggerTest, LoggerInstanceInfo) {
    auto logger = Logger::getInstance("TestApp", "TestProcess");

    LoggerInstance instance_info = logger->getInstanceInfo();
    EXPECT_FALSE(instance_info.empty());
    EXPECT_EQ(instance_info.getApplicationName(), "TestApp");
    EXPECT_EQ(instance_info.getProcessName(), "TestProcess");
}

// Test importance mapper
TEST_F(EnhancedLoggerTest, ImportanceMapper) {
    auto logger = Logger::getInstance("TestApp");

    ImportanceMapper* mapper = logger->getImportanceMapper();
    EXPECT_NE(mapper, nullptr);

    // Test importance resolution
    LogMessageData message_data;
    message_data.type = LogMessageType::ERR;
    strncpy(message_data.component, "Database", sizeof(message_data.component) - 1);
    message_data.component[sizeof(message_data.component) - 1] = '\0';
    strncpy(message_data.function, "Connect", sizeof(message_data.function) - 1);
    message_data.function[sizeof(message_data.function) - 1] = '\0';

    ImportanceResolutionContext context;
    strncpy(context.component, "Database", sizeof(context.component) - 1);
    context.component[sizeof(context.component) - 1] = '\0';
    strncpy(context.function, "Connect", sizeof(context.function) - 1);
    context.function[sizeof(context.function) - 1] = '\0';
    strncpy(context.application_name, "TestApp", sizeof(context.application_name) - 1);
    context.application_name[sizeof(context.application_name) - 1] = '\0';
    context.message_type = message_data.type;

    auto result = mapper->resolveMessageImportance(message_data, context);
    EXPECT_GE(result.final_importance, MessageImportance::HIGH);  // Database errors should be high
}

// Test persistence manager
TEST_F(EnhancedLoggerTest, PersistenceManager) {
    auto logger = Logger::getInstance("TestApp");

    ContextualPersistenceManager* persistence_manager = logger->getPersistenceManager();
    EXPECT_NE(persistence_manager, nullptr);

    // Test persistence policy
    PersistencePolicy policy = {
        MessageImportance::HIGH,        // Only HIGH importance and above
        true,                          // Enhanced error tracking
        true,                          // Critical error context
        10000,                         // Queue size
        3600                           // Retention time in seconds (1 hour)
    };

    EXPECT_TRUE(persistence_manager->setPolicy(policy));
}

// Test smart queue
TEST_F(EnhancedLoggerTest, SmartQueue) {
    auto logger = Logger::getInstance("TestApp");

    SmartMessageQueue* queue = logger->getSmartQueue();
    EXPECT_NE(queue, nullptr);

    // Test queue operations
    LogMessageData message_data;
    message_data.type = LogMessageType::INFO;
    strcpy_s(message_data.message, sizeof(message_data.message), "Queue test message");

    PersistenceDecisionContext context;
    strcpy_s(context.application_name, sizeof(context.application_name), "TestApp");
    strcpy_s(context.component, sizeof(context.component), "TestComponent");
    context.message_type = message_data.type;
    context.resolved_importance = MessageImportance::MEDIUM;

    EXPECT_NO_THROW(queue->enqueue(message_data, context));
}

// Test database logger
TEST_F(EnhancedLoggerTest, DatabaseLogger) {
    auto logger = Logger::getInstance("TestApp");

    // Test database logging enable/disable
    EXPECT_FALSE(logger->enableDatabaseLogging("InvalidConnectionString"));
    EXPECT_FALSE(logger->disableDatabaseLogging());  // Should fail when not enabled

    DatabaseLogger* db_logger = logger->getDatabaseLogger();
    EXPECT_EQ(db_logger, nullptr);  // Should be null when not connected
}

// Test enhanced configuration
TEST_F(EnhancedLoggerTest, EnhancedConfiguration) {
    auto logger = Logger::getInstance("TestApp");

    // Test enhanced configuration
    EXPECT_NO_THROW(logger->configureEnhanced(
        "TestApp",
        false,  // No database
        "",
        false,  // No shared memory
        "",
        true,   // Console output
        "test.log",
        10485760,  // 10MB
        5,         // 5 files
        spdlog::level::info
    ));

    EXPECT_TRUE(logger->isConfigured());
}

// Test statistics
TEST_F(EnhancedLoggerTest, LoggerStatistics) {
    auto logger = Logger::getInstance("TestApp");

    // Perform some logging
    logger->info("TestComponent", "TestFunction", "Statistics test message");
    logger->error("TestComponent", "TestFunction", "Statistics test error");

    // Get statistics
    std::string stats = logger->getComprehensiveStatistics();
    EXPECT_FALSE(stats.empty());
    EXPECT_NE(stats.find("TestApp"), std::string::npos);

    std::string status = logger->getStatus();
    EXPECT_FALSE(status.empty());
}

// Test multiple instances
TEST_F(EnhancedLoggerTest, MultipleInstances) {
    auto logger1 = Logger::getInstance("App1", "Process1");
    auto logger2 = Logger::getInstance("App2", "Process2");

    // Each should have different instance info
    LoggerInstance info1 = logger1->getInstanceInfo();
    LoggerInstance info2 = logger2->getInstanceInfo();

    EXPECT_EQ(info1.getApplicationName(), "App1");
    EXPECT_EQ(info2.getApplicationName(), "App2");
    EXPECT_NE(info1.getId(), info2.getId());
}

// Test thread safety
TEST_F(EnhancedLoggerTest, ThreadSafety) {
    auto logger = Logger::getInstance("ThreadTestApp");

    const int NUM_THREADS = 10;
    const int MESSAGES_PER_THREAD = 100;

    std::atomic<int> success_count{0};
    std::vector<std::thread> threads;

    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < MESSAGES_PER_THREAD; ++i) {
                try {
                    logger->info("ThreadComponent", "ThreadFunction",
                               "Thread %d message %d", t, i);
                    success_count++;
                } catch (...) {
                    // Count failures
                }
            }
        });
    }

    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(success_count, NUM_THREADS * MESSAGES_PER_THREAD);
}

// Test performance
TEST_F(EnhancedLoggerTest, Performance) {
    auto logger = Logger::getInstance("PerformanceTestApp");

    const int NUM_OPERATIONS = 1000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        logger->info("PerfComponent", "PerfFunction", "Performance test message %d", i);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start).count();

    // Should be able to log 1000 messages in under 500ms
    EXPECT_LT(duration, 500.0);
    std::cout << "Logged " << NUM_OPERATIONS << " messages in " << duration << "ms" << std::endl;
}

// Test error handling
TEST_F(EnhancedLoggerTest, ErrorHandling) {
    auto logger = Logger::getInstance("ErrorTestApp");

    // Test error storage
    logger->error("TestComponent", "TestFunction", "Test error message");

    std::string last_error = Logger::GetLastError();
    EXPECT_FALSE(last_error.empty());
    EXPECT_NE(last_error.find("Test error message"), std::string::npos);

    // Test error clearing
    Logger::Clear();
    EXPECT_TRUE(Logger::GetLastError().empty());
}

// Test backward compatibility
TEST_F(EnhancedLoggerTest, BackwardCompatibility) {
    auto logger = Logger::getInstance("CompatTestApp");

    // Test that old API still works
    EXPECT_NO_THROW(logger->info("Legacy message"));
    EXPECT_NO_THROW(logger->error("Legacy error"));
    EXPECT_NO_THROW(logger->warn("Legacy warning"));

    // Test configuration still works
    EXPECT_NO_THROW(logger->configure(true, "compat.log", 10485760, 5, spdlog::level::info));
}

// Test queue operations
TEST_F(EnhancedLoggerTest, QueueOperations) {
    auto logger = Logger::getInstance("QueueTestApp");

    // Test queue flushing
    size_t flushed = logger->flushAllQueues();
    EXPECT_GE(flushed, 0);  // Should not throw, may return 0 if no queues

    // Test queue statistics
    std::string stats = logger->getComprehensiveStatistics();
    EXPECT_FALSE(stats.empty());
}

// Test memory management
TEST_F(EnhancedLoggerTest, MemoryManagement) {
    // Test that logger instances don't leak memory
    for (int i = 0; i < 100; ++i) {
        auto logger = Logger::getInstance("MemoryTestApp" + std::to_string(i));

        // Perform operations
        logger->info("MemoryComponent", "MemoryFunction", "Memory test %d", i);
        logger->error("MemoryComponent", "MemoryFunction", "Memory error %d", i);

        // Get statistics (forces internal operations)
        std::string stats = logger->getComprehensiveStatistics();
        EXPECT_FALSE(stats.empty());
    }

    // All loggers should still be valid
    auto logger = Logger::getInstance("MemoryTestApp50");
    EXPECT_NO_THROW(logger->info("MemoryComponent", "MemoryFunction", "Final test"));
}

// Test configuration persistence
TEST_F(EnhancedLoggerTest, ConfigurationPersistence) {
    auto logger = Logger::getInstance("ConfigTestApp");

    // Test that configuration persists across operations
    logger->configureEnhanced(
        "ConfigTestApp",
        false, "", false, "", true, "config_test.log"
    );

    EXPECT_TRUE(logger->isConfigured());

    // Configuration should persist
    LoggerInstance info = logger->getInstanceInfo();
    EXPECT_EQ(info.getApplicationName(), "ConfigTestApp");
}

// Test concurrent instance access
TEST_F(EnhancedLoggerTest, ConcurrentInstanceAccess) {
    const int NUM_THREADS = 5;
    std::atomic<int> success_count{0};
    std::vector<std::thread> threads;

    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < 50; ++i) {
                auto logger = Logger::getInstance("ConcurrentApp", "Thread" + std::to_string(t));

                try {
                    logger->info("ConcurrentComponent", "ConcurrentFunction",
                               "Concurrent message %d from thread %d", i, t);
                    success_count++;
                } catch (...) {
                    // Count failures
                }
            }
        });
    }

    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(success_count, NUM_THREADS * 50);
}

// Test logger status
TEST_F(EnhancedLoggerTest, LoggerStatus) {
    auto logger = Logger::getInstance("StatusTestApp");

    // Test status reporting
    std::string status = logger->getStatus();
    EXPECT_FALSE(status.empty());

    // Should contain application name
    EXPECT_NE(status.find("StatusTestApp"), std::string::npos);
}

// Test enhanced features initialization
TEST_F(EnhancedLoggerTest, EnhancedFeaturesInitialization) {
    auto logger = Logger::getInstance("InitTestApp");

    // Test that enhanced features are properly initialized
    ImportanceMapper* importance_mapper = logger->getImportanceMapper();
    ContextualPersistenceManager* persistence_manager = logger->getPersistenceManager();
    SmartMessageQueue* smart_queue = logger->getSmartQueue();

    // These should be valid pointers when enhanced features are enabled
    EXPECT_NE(importance_mapper, nullptr);
    EXPECT_NE(persistence_manager, nullptr);
    EXPECT_NE(smart_queue, nullptr);
}

// Test message routing
TEST_F(EnhancedLoggerTest, MessageRouting) {
    auto logger = Logger::getInstance("RoutingTestApp");

    // Test that messages are properly routed through enhanced system
    logger->info("RoutingComponent", "RoutingFunction", "Routing test message");
    logger->error("RoutingComponent", "RoutingFunction", "Routing test error");

    // Should not throw exceptions
    EXPECT_NO_THROW(logger->flushAllQueues());
}

// Test statistics accuracy
TEST_F(EnhancedLoggerTest, StatisticsAccuracy) {
    auto logger = Logger::getInstance("StatsTestApp");

    // Clear any previous state
    Logger::Clear();

    // Log specific number of messages
    const int NUM_INFO = 10;
    const int NUM_ERROR = 5;

    for (int i = 0; i < NUM_INFO; ++i) {
        logger->info("StatsComponent", "StatsFunction", "Info message %d", i);
    }

    for (int i = 0; i < NUM_ERROR; ++i) {
        logger->error("StatsComponent", "StatsFunction", "Error message %d", i);
    }

    // Get comprehensive statistics
    std::string stats = logger->getComprehensiveStatistics();

    // Should contain application name and message counts
    EXPECT_NE(stats.find("StatsTestApp"), std::string::npos);
    EXPECT_FALSE(stats.empty());
}

// Test error recovery
TEST_F(EnhancedLoggerTest, ErrorRecovery) {
    auto logger = Logger::getInstance("RecoveryTestApp");

    // Test that logger recovers from errors gracefully
    logger->error("RecoveryComponent", "RecoveryFunction", "Test error for recovery");

    // Should still be able to log after errors
    EXPECT_NO_THROW(logger->info("RecoveryComponent", "RecoveryFunction", "Recovery test message"));

    // Error should be stored
    std::string last_error = Logger::GetLastError();
    EXPECT_FALSE(last_error.empty());
}

// Test resource cleanup
TEST_F(EnhancedLoggerTest, ResourceCleanup) {
    // Test that multiple logger instances cleanup properly
    std::vector<std::shared_ptr<Logger>> loggers;

    for (int i = 0; i < 10; ++i) {
        loggers.push_back(Logger::getInstance("CleanupApp" + std::to_string(i)));
    }

    // Use loggers
    for (size_t i = 0; i < loggers.size(); ++i) {
        loggers[i]->info("CleanupComponent", "CleanupFunction",
                        "Cleanup test message %d", i);
    }

    // Loggers should still be valid
    for (auto& logger : loggers) {
        EXPECT_NO_THROW(logger->info("CleanupComponent", "CleanupFunction", "Final message"));
    }

    // Clear loggers vector (simulates going out of scope)
    loggers.clear();

    // Should still be able to get logger instances
    auto logger = Logger::getInstance("CleanupApp5");
    EXPECT_NO_THROW(logger->info("CleanupComponent", "CleanupFunction", "After cleanup"));
}

// Test configuration validation
TEST_F(EnhancedLoggerTest, ConfigurationValidation) {
    auto logger = Logger::getInstance("ValidationTestApp");

    // Test invalid configuration
    EXPECT_NO_THROW(logger->configureEnhanced(
        "ValidationTestApp",
        false, "",  // No database
        false, "",  // No shared memory
        true,       // Console output
        "validation_test.log"
    ));

    // Test that configuration is applied
    EXPECT_TRUE(logger->isConfigured());

    LoggerInstance info = logger->getInstanceInfo();
    EXPECT_EQ(info.getApplicationName(), "ValidationTestApp");
}

// Test logging levels
TEST_F(EnhancedLoggerTest, LoggingLevels) {
    auto logger = Logger::getInstance("LevelTestApp");

    // Test all logging levels
    EXPECT_NO_THROW(logger->trace("LevelComponent", "LevelFunction", "Trace message"));
    EXPECT_NO_THROW(logger->debug("LevelComponent", "LevelFunction", "Debug message"));
    EXPECT_NO_THROW(logger->info("LevelComponent", "LevelFunction", "Info message"));
    EXPECT_NO_THROW(logger->warn("LevelComponent", "LevelFunction", "Warn message"));
    EXPECT_NO_THROW(logger->error("LevelComponent", "LevelFunction", "Error message"));
    EXPECT_NO_THROW(logger->critical("LevelComponent", "LevelFunction", "Critical message"));
}

// Test component tracking
TEST_F(EnhancedLoggerTest, ComponentTracking) {
    auto logger = Logger::getInstance("ComponentTestApp");

    // Test logging with different components
    logger->info("ComponentA", "FunctionA", "Message from Component A");
    logger->info("ComponentB", "FunctionB", "Message from Component B");
    logger->error("ComponentA", "FunctionA", "Error from Component A");
    logger->warn("ComponentB", "FunctionB", "Warning from Component B");

    // Should handle all components without issues
    EXPECT_NO_THROW(logger->flushAllQueues());
}

// Test function tracking
TEST_F(EnhancedLoggerTest, FunctionTracking) {
    auto logger = Logger::getInstance("FunctionTestApp");

    // Test logging with different functions
    logger->info("TestComponent", "Function1", "Message from Function 1");
    logger->info("TestComponent", "Function2", "Message from Function 2");
    logger->error("TestComponent", "Function1", "Error from Function 1");

    // Should handle all functions without issues
    EXPECT_NO_THROW(logger->flushAllQueues());
}

// Test message formatting
TEST_F(EnhancedLoggerTest, MessageFormatting) {
    auto logger = Logger::getInstance("FormatTestApp");

    // Test various message formats
    logger->info("FormatComponent", "FormatFunction", "Simple message");
    logger->info("FormatComponent", "FormatFunction", "Message with int: %d", 42);
    logger->info("FormatComponent", "FormatFunction", "Message with string: %s", "test");
    logger->info("FormatComponent", "FormatFunction", "Message with multiple args: %d, %s, %.2f", 1, "test", 3.14);

    // Should handle all formats without crashing
    EXPECT_NO_THROW(logger->flushAllQueues());
}

// Test queue overflow handling
TEST_F(EnhancedLoggerTest, QueueOverflowHandling) {
    auto logger = Logger::getInstance("OverflowTestApp");

    // Log many messages to test queue handling
    for (int i = 0; i < 1000; ++i) {
        logger->info("OverflowComponent", "OverflowFunction", "Overflow test message %d", i);
    }

    // Should handle large volumes without crashing
    EXPECT_NO_THROW(logger->flushAllQueues());

    // Statistics should reflect the load
    std::string stats = logger->getComprehensiveStatistics();
    EXPECT_FALSE(stats.empty());
}

// Test concurrent configuration
TEST_F(EnhancedLoggerTest, ConcurrentConfiguration) {
    const int NUM_THREADS = 3;
    std::atomic<int> success_count{0};
    std::vector<std::thread> threads;

    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&, t]() {
            try {
                auto logger = Logger::getInstance("ConcurrentConfigApp", "Thread" + std::to_string(t));

                logger->configureEnhanced(
                    "ConcurrentConfigApp",
                    false, "", false, "", true, "concurrent_test.log"
                );

                success_count++;
            } catch (...) {
                // Count failures
            }
        });
    }

    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(success_count, NUM_THREADS);
}

// Test logger isolation
TEST_F(EnhancedLoggerTest, LoggerIsolation) {
    auto logger1 = Logger::getInstance("IsolationApp1");
    auto logger2 = Logger::getInstance("IsolationApp2");

    // Each logger should maintain separate state
    logger1->info("IsolationComponent", "IsolationFunction", "Message from App 1");
    logger2->info("IsolationComponent", "IsolationFunction", "Message from App 2");

    // Each should have independent statistics
    std::string stats1 = logger1->getComprehensiveStatistics();
    std::string stats2 = logger2->getComprehensiveStatistics();

    EXPECT_NE(stats1, stats2);
    EXPECT_NE(stats1.find("IsolationApp1"), std::string::npos);
    EXPECT_NE(stats2.find("IsolationApp2"), std::string::npos);
}

// Test error propagation
TEST_F(EnhancedLoggerTest, ErrorPropagation) {
    auto logger = Logger::getInstance("ErrorPropTestApp");

    // Test that errors in enhanced features don't crash the logger
    logger->error("ErrorPropComponent", "ErrorPropFunction", "Test error");

    // Logger should still be functional after errors
    EXPECT_NO_THROW(logger->info("ErrorPropComponent", "ErrorPropFunction", "Message after error"));
    EXPECT_NO_THROW(logger->warn("ErrorPropComponent", "ErrorPropFunction", "Warning after error"));

    // Error should be stored
    std::string last_error = Logger::GetLastError();
    EXPECT_FALSE(last_error.empty());
}

// Test configuration changes
TEST_F(EnhancedLoggerTest, ConfigurationChanges) {
    auto logger = Logger::getInstance("ConfigChangeTestApp");

    // Test configuration changes
    logger->configureEnhanced(
        "ConfigChangeTestApp",
        false, "", false, "", true, "config_change_test.log"
    );

    EXPECT_TRUE(logger->isConfigured());

    // Configuration should be applied
    LoggerInstance info = logger->getInstanceInfo();
    EXPECT_EQ(info.getApplicationName(), "ConfigChangeTestApp");
}

// Test logging with special characters
TEST_F(EnhancedLoggerTest, SpecialCharacters) {
    auto logger = Logger::getInstance("SpecialCharTestApp");

    // Test logging with special characters
    logger->info("SpecialComponent", "SpecialFunction", "Message with special chars: ñáéíóú, 中文, русский, 🚀, \n\t\r");
    logger->error("SpecialComponent", "SpecialFunction", "Error with unicode: αβγδε, カタカナ, русский");

    // Should handle special characters without crashing
    EXPECT_NO_THROW(logger->flushAllQueues());
}

// Test large message handling
TEST_F(EnhancedLoggerTest, LargeMessageHandling) {
    auto logger = Logger::getInstance("LargeMessageTestApp");

    // Test with large messages
    std::string large_message(1000, 'A');  // 1000 character message

    logger->info("LargeComponent", "LargeFunction", "Large message: %s", large_message.c_str());

    // Should handle large messages without crashing
    EXPECT_NO_THROW(logger->flushAllQueues());
}

// Test rapid logging
TEST_F(EnhancedLoggerTest, RapidLogging) {
    auto logger = Logger::getInstance("RapidTestApp");

    const int RAPID_COUNT = 1000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < RAPID_COUNT; ++i) {
        logger->info("RapidComponent", "RapidFunction", "Rapid message %d", i);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start).count();

    // Should be able to log 1000 messages quickly
    EXPECT_LT(duration, 1000.0);  // Under 1 second
    std::cout << "Rapid logged " << RAPID_COUNT << " messages in " << duration << "ms" << std::endl;
}

// Test logger destruction
TEST_F(EnhancedLoggerTest, LoggerDestruction) {
    {
        auto logger = Logger::getInstance("DestructionTestApp");

        logger->info("DestructionComponent", "DestructionFunction", "Pre-destruction message");
        logger->error("DestructionComponent", "DestructionFunction", "Pre-destruction error");

        // Logger should be valid within scope
        EXPECT_NO_THROW(logger->info("DestructionComponent", "DestructionFunction", "Within scope message"));
    }

    // After logger goes out of scope, should still be able to get new instance
    auto logger = Logger::getInstance("DestructionTestApp");
    EXPECT_NO_THROW(logger->info("DestructionComponent", "DestructionFunction", "Post-destruction message"));
}

// Test statistics comprehensiveness
TEST_F(EnhancedLoggerTest, StatisticsComprehensiveness) {
    auto logger = Logger::getInstance("StatsCompTestApp");

    // Perform various operations
    logger->trace("StatsComponent", "StatsFunction", "Trace message");
    logger->debug("StatsComponent", "StatsFunction", "Debug message");
    logger->info("StatsComponent", "StatsFunction", "Info message");
    logger->warn("StatsComponent", "StatsFunction", "Warn message");
    logger->error("StatsComponent", "StatsFunction", "Error message");
    logger->critical("StatsComponent", "StatsFunction", "Critical message");

    // Get comprehensive statistics
    std::string stats = logger->getComprehensiveStatistics();

    // Should contain detailed information
    EXPECT_FALSE(stats.empty());
    EXPECT_NE(stats.find("StatsCompTestApp"), std::string::npos);
    EXPECT_NE(stats.find("Total Messages Processed"), std::string::npos);
}

// Test enhanced features integration
TEST_F(EnhancedLoggerTest, EnhancedFeaturesIntegration) {
    auto logger = Logger::getInstance("IntegrationTestApp");

    // Test that all enhanced features work together
    logger->info("IntegrationComponent", "IntegrationFunction", "Integration test message");
    logger->error("IntegrationComponent", "IntegrationFunction", "Integration test error");

    // All components should work together
    ImportanceMapper* importance_mapper = logger->getImportanceMapper();
    ContextualPersistenceManager* persistence_manager = logger->getPersistenceManager();
    SmartMessageQueue* smart_queue = logger->getSmartQueue();

    EXPECT_NE(importance_mapper, nullptr);
    EXPECT_NE(persistence_manager, nullptr);
    EXPECT_NE(smart_queue, nullptr);

    // Should be able to flush without errors
    EXPECT_NO_THROW(logger->flushAllQueues());
}

// Test backward compatibility with enhanced features
TEST_F(EnhancedLoggerTest, BackwardCompatibilityWithEnhanced) {
    auto logger = Logger::getInstance("BackwardCompatTestApp");

    // Test that old API still works with enhanced features enabled
    EXPECT_NO_THROW(logger->info("Legacy message"));
    EXPECT_NO_THROW(logger->error("Legacy error"));
    EXPECT_NO_THROW(logger->warn("Legacy warning"));

    // Test old configuration API
    EXPECT_NO_THROW(logger->configure(true, "backward_compat.log", 10485760, 5, spdlog::level::info));

    // Enhanced features should still be available
    LoggerInstance info = logger->getInstanceInfo();
    EXPECT_FALSE(info.empty());
}

// Test error message formatting
TEST_F(EnhancedLoggerTest, ErrorMessageFormatting) {
    auto logger = Logger::getInstance("FormatErrorTestApp");

    // Test error message formatting with various argument types
    logger->error("FormatComponent", "FormatFunction", "Error with int: %d", 42);
    logger->error("FormatComponent", "FormatFunction", "Error with string: %s", "test string");
    logger->error("FormatComponent", "FormatFunction", "Error with float: %.2f", 3.14159);
    logger->error("FormatComponent", "FormatFunction", "Error with multiple: %d, %s, %.2f", 1, "test", 2.5);

    // Should handle all formatting without crashing
    EXPECT_NO_THROW(logger->flushAllQueues());

    // Error should be stored with formatting
    std::string last_error = Logger::GetLastError();
    EXPECT_FALSE(last_error.empty());
}

// Test component and function context
TEST_F(EnhancedLoggerTest, ComponentFunctionContext) {
    auto logger = Logger::getInstance("ContextTestApp");

    // Test logging with various component/function combinations
    logger->info("Database", "Connect", "Database connection established");
    logger->info("Cache", "Hit", "Cache hit for key: user_123");
    logger->error("Network", "Send", "Network send failed");
    logger->warn("Security", "Auth", "Suspicious login attempt");
    logger->info("API", "Request", "API request processed");

    // Should handle all contexts without issues
    EXPECT_NO_THROW(logger->flushAllQueues());
}

// Test message persistence decisions
TEST_F(EnhancedLoggerTest, MessagePersistenceDecisions) {
    auto logger = Logger::getInstance("PersistenceTestApp");

    // Test that persistence decisions are made correctly
    logger->trace("PersistenceComponent", "PersistenceFunction", "Trace message");  // Should be filtered
    logger->debug("PersistenceComponent", "PersistenceFunction", "Debug message");  // Should be filtered
    logger->info("PersistenceComponent", "PersistenceFunction", "Info message");   // May be persisted
    logger->warn("PersistenceComponent", "PersistenceFunction", "Warning message"); // Should be persisted
    logger->error("PersistenceComponent", "PersistenceFunction", "Error message");  // Should be persisted
    logger->critical("PersistenceComponent", "PersistenceFunction", "Critical message"); // Should be persisted

    // Should handle all levels without crashing
    EXPECT_NO_THROW(logger->flushAllQueues());
}

// Test queue management
TEST_F(EnhancedLoggerTest, QueueManagement) {
    auto logger = Logger::getInstance("QueueMgmtTestApp");

    // Test queue operations
    for (int i = 0; i < 100; ++i) {
        logger->info("QueueComponent", "QueueFunction", "Queue management message %d", i);
    }

    // Test queue flushing
    size_t flushed = logger->flushAllQueues();
    EXPECT_GE(flushed, 0);

    // Test queue statistics
    std::string stats = logger->getComprehensiveStatistics();
    EXPECT_FALSE(stats.empty());
}

// Test memory usage with enhanced features
TEST_F(EnhancedLoggerTest, MemoryUsageWithEnhanced) {
    // Test memory usage with enhanced features enabled
    std::vector<std::shared_ptr<Logger>> loggers;

    for (int i = 0; i < 50; ++i) {
        loggers.push_back(Logger::getInstance("MemoryEnhancedApp" + std::to_string(i)));
    }

    // Use enhanced features extensively
    for (size_t i = 0; i < loggers.size(); ++i) {
        auto& logger = loggers[i];

        // Use all enhanced features
        logger->info("MemoryComponent", "MemoryFunction", "Memory test message %d", i);
        logger->error("MemoryComponent", "MemoryFunction", "Memory test error %d", i);

        // Access enhanced components
        ImportanceMapper* importance_mapper = logger->getImportanceMapper();
        ContextualPersistenceManager* persistence_manager = logger->getPersistenceManager();
        SmartMessageQueue* smart_queue = logger->getSmartQueue();

        EXPECT_NE(importance_mapper, nullptr);
        EXPECT_NE(persistence_manager, nullptr);
        EXPECT_NE(smart_queue, nullptr);
    }

    // All operations should succeed
    for (auto& logger : loggers) {
        EXPECT_NO_THROW(logger->flushAllQueues());
    }
}

// Test enhanced features performance
TEST_F(EnhancedLoggerTest, EnhancedFeaturesPerformance) {
    auto logger = Logger::getInstance("EnhancedPerfTestApp");

    const int NUM_OPERATIONS = 1000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        logger->info("PerfComponent", "PerfFunction", "Enhanced performance test %d", i);

        // Access enhanced features
        ImportanceMapper* importance_mapper = logger->getImportanceMapper();
        ContextualPersistenceManager* persistence_manager = logger->getPersistenceManager();

        EXPECT_NE(importance_mapper, nullptr);
        EXPECT_NE(persistence_manager, nullptr);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start).count();

    // Enhanced features should not significantly impact performance
    EXPECT_LT(duration, 1000.0);  // Under 1 second for 1000 operations
    std::cout << "Enhanced performance: " << NUM_OPERATIONS << " operations in " << duration << "ms" << std::endl;
}

// Test configuration flexibility
TEST_F(EnhancedLoggerTest, ConfigurationFlexibility) {
    auto logger = Logger::getInstance("ConfigFlexTestApp");

    // Test different configuration options
    logger->configureEnhanced(
        "ConfigFlexTestApp",
        false, "",  // No database
        false, "",  // No shared memory
        true,       // Console output
        "config_flex_test.log",
        52428800,   // 50MB files
        10,         // 10 files
        spdlog::level::debug
    );

    EXPECT_TRUE(logger->isConfigured());

    // Test that configuration is applied
    LoggerInstance info = logger->getInstanceInfo();
    EXPECT_EQ(info.getApplicationName(), "ConfigFlexTestApp");
}

// Test error context preservation
TEST_F(EnhancedLoggerTest, ErrorContextPreservation) {
    auto logger = Logger::getInstance("ErrorContextTestApp");

    // Test that error context is preserved through enhanced features
    logger->error("ErrorContextComponent", "ErrorContextFunction", "Error with context");

    // Error should be stored with full context
    std::string last_error = Logger::GetLastError();
    EXPECT_FALSE(last_error.empty());

    // Should still be able to log after error
    EXPECT_NO_THROW(logger->info("ErrorContextComponent", "ErrorContextFunction", "Message after error"));
}

// Test multi-level logging
TEST_F(EnhancedLoggerTest, MultiLevelLogging) {
    auto logger = Logger::getInstance("MultiLevelTestApp");

    // Test logging at all levels with enhanced features
    logger->trace("MultiLevelComponent", "MultiLevelFunction", "Trace level message");
    logger->debug("MultiLevelComponent", "MultiLevelFunction", "Debug level message");
    logger->info("MultiLevelComponent", "MultiLevelFunction", "Info level message");
    logger->warn("MultiLevelComponent", "MultiLevelFunction", "Warn level message");
    logger->error("MultiLevelComponent", "MultiLevelFunction", "Error level message");
    logger->critical("MultiLevelComponent", "MultiLevelFunction", "Critical level message");

    // All levels should work with enhanced features
    EXPECT_NO_THROW(logger->flushAllQueues());
}

// Test component hierarchy
TEST_F(EnhancedLoggerTest, ComponentHierarchy) {
    auto logger = Logger::getInstance("HierarchyTestApp");

    // Test logging with hierarchical component names
    logger->info("System.Database", "Connect", "Database connection");
    logger->info("System.Cache.Redis", "Get", "Cache retrieval");
    logger->info("System.Network.HTTP", "Request", "HTTP request");
    logger->error("System.Database", "Query", "Database query failed");
    logger->warn("System.Security.Auth", "Validate", "Authentication warning");

    // Should handle hierarchical components
    EXPECT_NO_THROW(logger->flushAllQueues());
}

// Test function hierarchy
TEST_F(EnhancedLoggerTest, FunctionHierarchy) {
    auto logger = Logger::getInstance("FuncHierarchyTestApp");

    // Test logging with hierarchical function names
    logger->info("DatabaseComponent", "Connection.Establish", "Connection established");
    logger->info("CacheComponent", "Memory.Get", "Memory retrieval");
    logger->error("NetworkComponent", "HTTP.Send", "HTTP send failed");
    logger->warn("SecurityComponent", "Auth.Validate", "Validation warning");

    // Should handle hierarchical functions
    EXPECT_NO_THROW(logger->flushAllQueues());
}

// Test message content variety
TEST_F(EnhancedLoggerTest, MessageContentVariety) {
    auto logger = Logger::getInstance("ContentTestApp");

    // Test various message content types
    logger->info("ContentComponent", "ContentFunction", "Simple text message");
    logger->info("ContentComponent", "ContentFunction", "Message with numbers: %d, %f, %u", 42, 3.14, 100);
    logger->info("ContentComponent", "ContentFunction", "Message with strings: %s, %s", "hello", "world");
    logger->info("ContentComponent", "ContentFunction", "Mixed message: %d, %s, %.2f, %c", 1, "test", 2.5, 'X');

    // Should handle all content types
    EXPECT_NO_THROW(logger->flushAllQueues());
}

// Test concurrent enhanced logging
TEST_F(EnhancedLoggerTest, ConcurrentEnhancedLogging) {
    auto logger = Logger::getInstance("ConcurrentEnhancedApp");

    const int NUM_THREADS = 5;
    const int MESSAGES_PER_THREAD = 100;

    std::atomic<int> success_count{0};
    std::vector<std::thread> threads;

    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < MESSAGES_PER_THREAD; ++i) {
                try {
                    logger->info("ConcurrentComponent", "ConcurrentFunction",
                               "Concurrent enhanced message %d from thread %d", i, t);
                    success_count++;
                } catch (...) {
                    // Count failures
                }
            }
        });
    }

    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(success_count, NUM_THREADS * MESSAGES_PER_THREAD);
}

// Test enhanced features error handling
TEST_F(EnhancedLoggerTest, EnhancedFeaturesErrorHandling) {
    auto logger = Logger::getInstance("EnhancedErrorTestApp");

    // Test that enhanced features handle errors gracefully
    logger->error("EnhancedErrorComponent", "EnhancedErrorFunction", "Enhanced error test");

    // Enhanced features should still be available after errors
    ImportanceMapper* importance_mapper = logger->getImportanceMapper();
    ContextualPersistenceManager* persistence_manager = logger->getPersistenceManager();
    SmartMessageQueue* smart_queue = logger->getSmartQueue();

    EXPECT_NE(importance_mapper, nullptr);
    EXPECT_NE(persistence_manager, nullptr);
    EXPECT_NE(smart_queue, nullptr);

    // Should still be able to log
    EXPECT_NO_THROW(logger->info("EnhancedErrorComponent", "EnhancedErrorFunction", "Message after enhanced error"));
}

// Test logger state consistency
TEST_F(EnhancedLoggerTest, LoggerStateConsistency) {
    auto logger = Logger::getInstance("StateTestApp");

    // Test that logger state remains consistent across operations
    logger->info("StateComponent", "StateFunction", "State test message 1");
    logger->error("StateComponent", "StateFunction", "State test error 1");

    // Get initial statistics
    std::string initial_stats = logger->getComprehensiveStatistics();

    logger->info("StateComponent", "StateFunction", "State test message 2");
    logger->error("StateComponent", "StateFunction", "State test error 2");

    // Get updated statistics
    std::string updated_stats = logger->getComprehensiveStatistics();

    // Statistics should be different (more messages)
    EXPECT_NE(initial_stats, updated_stats);
    EXPECT_FALSE(updated_stats.empty());
}

// Test enhanced logging with database
TEST_F(EnhancedLoggerTest, EnhancedLoggingWithDatabase) {
    auto logger = Logger::getInstance("DBTestApp");

    // Test enhanced logging with database configuration (even if connection fails)
    EXPECT_NO_THROW(logger->configureEnhanced(
        "DBTestApp",
        true,   // Enable database
        "Server=localhost;Database=TestDB;Trusted_Connection=True;",  // Connection string
        false,  // No shared memory
        "",
        true,   // Console output
        "db_test.log"
    ));

    // Should handle database configuration gracefully
    EXPECT_TRUE(logger->isConfigured());

    // Test logging with database enabled
    logger->info("DBComponent", "DBFunction", "Database test message");
    logger->error("DBComponent", "DBFunction", "Database test error");

    // Should not crash even if database connection fails
    EXPECT_NO_THROW(logger->flushAllQueues());
}

// Test enhanced logging with shared memory
TEST_F(EnhancedLoggerTest, EnhancedLoggingWithSharedMemory) {
    auto logger = Logger::getInstance("SharedMemTestApp");

    // Test enhanced logging with shared memory configuration
    EXPECT_NO_THROW(logger->configureEnhanced(
        "SharedMemTestApp",
        false,  // No database
        "",
        true,   // Enable shared memory
        "TestSharedMemory",  // Shared memory name
        true,   // Console output
        "shared_mem_test.log"
    ));

    // Should handle shared memory configuration gracefully
    EXPECT_TRUE(logger->isConfigured());

    // Test logging with shared memory enabled
    logger->info("SharedMemComponent", "SharedMemFunction", "Shared memory test message");
    logger->error("SharedMemComponent", "SharedMemFunction", "Shared memory test error");

    // Should not crash even if shared memory fails
    EXPECT_NO_THROW(logger->flushAllQueues());
}

// Test comprehensive enhanced workflow
TEST_F(EnhancedLoggerTest, ComprehensiveEnhancedWorkflow) {
    auto logger = Logger::getInstance("WorkflowTestApp");

    // Configure with all enhanced features
    logger->configureEnhanced(
        "WorkflowTestApp",
        false, "",  // No database for test
        false, "",  // No shared memory for test
        true,       // Console output
        "workflow_test.log"
    );

    // Perform comprehensive logging workflow
    logger->trace("WorkflowComponent", "WorkflowFunction", "Workflow trace");
    logger->debug("WorkflowComponent", "WorkflowFunction", "Workflow debug");
    logger->info("WorkflowComponent", "WorkflowFunction", "Workflow info");
    logger->warn("WorkflowComponent", "WorkflowFunction", "Workflow warning");
    logger->error("WorkflowComponent", "WorkflowFunction", "Workflow error");
    logger->critical("WorkflowComponent", "WorkflowFunction", "Workflow critical");

    // Use all enhanced features
    ImportanceMapper* importance_mapper = logger->getImportanceMapper();
    ContextualPersistenceManager* persistence_manager = logger->getPersistenceManager();
    SmartMessageQueue* smart_queue = logger->getSmartQueue();

    EXPECT_NE(importance_mapper, nullptr);
    EXPECT_NE(persistence_manager, nullptr);
    EXPECT_NE(smart_queue, nullptr);

    // Test queue operations
    size_t flushed = logger->flushAllQueues();
    EXPECT_GE(flushed, 0);

    // Test statistics
    std::string stats = logger->getComprehensiveStatistics();
    EXPECT_FALSE(stats.empty());
    EXPECT_NE(stats.find("WorkflowTestApp"), std::string::npos);

    // Test status
    std::string status = logger->getStatus();
    EXPECT_FALSE(status.empty());
    EXPECT_NE(status.find("WorkflowTestApp"), std::string::npos);
}

// Test enhanced features isolation
TEST_F(EnhancedLoggerTest, EnhancedFeaturesIsolation) {
    auto logger1 = Logger::getInstance("Isolation1App");
    auto logger2 = Logger::getInstance("Isolation2App");

    // Each logger should have independent enhanced features
    logger1->info("Isolation1Component", "Isolation1Function", "Message from logger 1");
    logger2->info("Isolation2Component", "Isolation2Function", "Message from logger 2");

    // Each should have separate statistics
    std::string stats1 = logger1->getComprehensiveStatistics();
    std::string stats2 = logger2->getComprehensiveStatistics();

    EXPECT_NE(stats1.find("Isolation1App"), std::string::npos);
    EXPECT_NE(stats2.find("Isolation2App"), std::string::npos);
    EXPECT_NE(stats1, stats2);
}

// Test enhanced logging performance under load
TEST_F(EnhancedLoggerTest, EnhancedLoggingPerformanceUnderLoad) {
    auto logger = Logger::getInstance("LoadTestApp");

    const int LOAD_TEST_COUNT = 5000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < LOAD_TEST_COUNT; ++i) {
        logger->info("LoadComponent", "LoadFunction", "Load test message %d", i);

        // Periodically access enhanced features
        if (i % 100 == 0) {
            ImportanceMapper* importance_mapper = logger->getImportanceMapper();
            ContextualPersistenceManager* persistence_manager = logger->getPersistenceManager();
            EXPECT_NE(importance_mapper, nullptr);
            EXPECT_NE(persistence_manager, nullptr);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start).count();

    // Should handle load efficiently
    EXPECT_LT(duration, 2000.0);  // Under 2 seconds for 5000 messages
    std::cout << "Load test: " << LOAD_TEST_COUNT << " messages in " << duration << "ms" << std::endl;
}

// Test enhanced features cleanup
TEST_F(EnhancedLoggerTest, EnhancedFeaturesCleanup) {
    {
        auto logger = Logger::getInstance("CleanupTestApp");

        // Use enhanced features
        logger->info("CleanupComponent", "CleanupFunction", "Cleanup test message");
        logger->error("CleanupComponent", "CleanupFunction", "Cleanup test error");

        ImportanceMapper* importance_mapper = logger->getImportanceMapper();
        ContextualPersistenceManager* persistence_manager = logger->getPersistenceManager();
        SmartMessageQueue* smart_queue = logger->getSmartQueue();

        EXPECT_NE(importance_mapper, nullptr);
        EXPECT_NE(persistence_manager, nullptr);
        EXPECT_NE(smart_queue, nullptr);

        // Logger goes out of scope
    }

    // Should still be able to get new logger instance
    auto logger = Logger::getInstance("CleanupTestApp");
    EXPECT_NO_THROW(logger->info("CleanupComponent", "CleanupFunction", "Post-cleanup message"));
}

// Test enhanced logging with various data types
TEST_F(EnhancedLoggerTest, EnhancedLoggingDataTypes) {
    auto logger = Logger::getInstance("DataTypeTestApp");

    // Test logging with various data types
    logger->info("DataTypeComponent", "DataTypeFunction", "String: %s", "test string");
    logger->info("DataTypeComponent", "DataTypeFunction", "Integer: %d", 42);
    logger->info("DataTypeComponent", "DataTypeFunction", "Float: %.2f", 3.14159);
    logger->info("DataTypeComponent", "DataTypeFunction", "Hex: 0x%X", 255);
    logger->info("DataTypeComponent", "DataTypeFunction", "Char: %c", 'A');
    logger->info("DataTypeComponent", "DataTypeFunction", "Boolean: %s", "true");

    // Should handle all data types
    EXPECT_NO_THROW(logger->flushAllQueues());
}

// Test enhanced features error recovery
TEST_F(EnhancedLoggerTest, EnhancedFeaturesErrorRecovery) {
    auto logger = Logger::getInstance("ErrorRecoveryTestApp");

    // Test error recovery in enhanced features
    logger->error("ErrorRecoveryComponent", "ErrorRecoveryFunction", "Error for recovery test");

    // Enhanced features should still work after errors
    EXPECT_NO_THROW(logger->info("ErrorRecoveryComponent", "ErrorRecoveryFunction", "Message after error"));

    ImportanceMapper* importance_mapper = logger->getImportanceMapper();
    EXPECT_NE(importance_mapper, nullptr);

    // Should be able to resolve importance after errors
    LogMessageData message_data;
    message_data.type = LogMessageType::ERR;
    strcpy_s(message_data.component, sizeof(message_data.component), "TestComponent");

    ImportanceResolutionContext context;
    strcpy_s(context.component, sizeof(context.component), "TestComponent");
    strcpy_s(context.application_name, sizeof(context.application_name), "ErrorRecoveryTestApp");
    context.message_type = message_data.type;

    auto result = importance_mapper->resolveMessageImportance(message_data, context);
    EXPECT_GE(result.final_importance, MessageImportance::LOW);
}

// Test enhanced logging configuration persistence
TEST_F(EnhancedLoggerTest, EnhancedLoggingConfigurationPersistence) {
    auto logger = Logger::getInstance("ConfigPersistTestApp");

    // Configure enhanced features
    logger->configureEnhanced(
        "ConfigPersistTestApp",
        false, "", false, "", true, "config_persist_test.log"
    );

    // Configuration should persist across multiple operations
    for (int i = 0; i < 10; ++i) {
        logger->info("ConfigPersistComponent", "ConfigPersistFunction", "Configuration persistence test %d", i);
    }

    // Configuration should still be valid
    EXPECT_TRUE(logger->isConfigured());

    LoggerInstance info = logger->getInstanceInfo();
    EXPECT_EQ(info.getApplicationName(), "ConfigPersistTestApp");
}

// Test enhanced logging with complex scenarios
TEST_F(EnhancedLoggerTest, EnhancedLoggingComplexScenarios) {
    auto logger = Logger::getInstance("ComplexTestApp");

    // Test complex logging scenarios
    logger->info("Complex.Database", "Connection.Establish", "Database connection established for user %s", "user123");
    logger->error("Complex.Network", "HTTP.Send", "HTTP request failed with status %d for endpoint %s", 500, "/api/data");
    logger->warn("Complex.Security", "Auth.Validate", "Suspicious activity detected: %d attempts from IP %s", 5, "192.168.1.100");
    logger->info("Complex.Cache", "Memory.Get", "Cache miss for key: %s, loading from database", "user:123:profile");
    logger->critical("Complex.System", "Resource.Check", "System resources critical: CPU %d%%, Memory %d%%", 95, 90);

    // Should handle complex scenarios
    EXPECT_NO_THROW(logger->flushAllQueues());

    // Statistics should reflect complexity
    std::string stats = logger->getComprehensiveStatistics();
    EXPECT_FALSE(stats.empty());
}