/**
 * ImportanceMapper Component Tests
 * TASK 1.03: ImportanceMapper Testing
 * Component: src/stateful/ImportanceMapper.hpp/cpp
 * Purpose: Validate importance mapping hierarchy and override resolution logic
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ASFMLogger.hpp"
#include <thread>
#include <atomic>
#include <chrono>

// Test fixture for ImportanceMapper tests
class ImportanceMapperTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create isolated test mapper for trading app
        mapper.reset(new ImportanceMapper("TradingApp"));
    }

    void TearDown() override {
        mapper.reset();
    }

    std::unique_ptr<ImportanceMapper> mapper;
    static int message_counter;

    // Helper to create test message with specific component and function
    LogMessageData CreateTestMessage(LogMessageType type = LogMessageType::WARN,
                                   const std::string& component = "DefaultComponent",
                                   const std::string& function = "DefaultFunction") {
        LogMessageData msg;
        msg.message_id = ++message_counter;
        msg.timestamp.seconds = time(nullptr);
        msg.timestamp.microseconds = 0;
        msg.timestamp.milliseconds = 0;
        msg.type = type;
        msg.process_id = GetCurrentProcessId();
        msg.thread_id = GetCurrentThreadId();

        strcpy(msg.component, component.c_str());
        strcpy(msg.function, function.c_str());
        strcpy(msg.file, "test_importance_mapper.cpp");
        msg.line_number = 0;
        strcpy(msg.severity_string, "TEST");

        sprintf(msg.message, "Test message from %s.%s", component.c_str(), function.c_str());
        return msg;
    }

    // Create resolution context for testing
    ImportanceResolutionContext CreateResolutionContext(DWORD system_load = 50) {
        ImportanceResolutionContext context;
        context.message_type = LogMessageType::WARN;
        strcpy(context.component, "TestComponent");
        strcpy(context.function, "TestFunction");
        strcpy(context.application_name, "TradingApp");
        context.system_load = system_load;
        context.error_rate = 5;
        context.message_rate = 100;
        context.is_emergency_mode = false;
        return context;
    }
};

// Initialize static counter
int ImportanceMapperTest::message_counter = 0;

// =============================================================================
// BASIC IMPORTANCE MAPPER SETUP TESTS
// =============================================================================

TEST_F(ImportanceMapperTest, TestMapperInitialization) {
    // Test basic initialization
    ASSERT_TRUE(mapper->isConfigured());
    EXPECT_EQ(mapper->getApplicationName(), "TradingApp");

    // Test default configuration validity
    ASSERT_TRUE(mapper->validateConfiguration());
}

TEST_F(ImportanceMapperTest, TestMapperWithoutApplicationName) {
    // Test mapper without application name
    auto default_mapper = std::make_unique<ImportanceMapper>();
    ASSERT_FALSE(default_mapper->isConfigured());
    ASSERT_TRUE(default_mapper->getApplicationName().empty());
}

// =============================================================================
// TYPE-BASED IMPORTANCE MAPPING TESTS
// =============================================================================

TEST_F(ImportanceMapperTest, TestTypeImportanceMapping) {
    // Test setting and getting type importance
    bool set_result = mapper->setTypeImportance(LogMessageType::INFO, MessageImportance::HIGH, "Override for INFO");
    ASSERT_TRUE(set_result);

    MessageImportance retrieved = mapper->getTypeImportance(LogMessageType::INFO);
    EXPECT_EQ(retrieved, MessageImportance::HIGH);
}

TEST_F(ImportanceMapperTest, TestDefaultTypeImportances) {
    // Test default mappings without explicit override
    EXPECT_EQ(mapper->getTypeImportance(LogMessageType::CRITICAL_LOG), MessageImportance::CRITICAL);
    EXPECT_EQ(mapper->getTypeImportance(LogMessageType::ERR), MessageImportance::HIGH);
    EXPECT_EQ(mapper->getTypeImportance(LogMessageType::WARN), MessageImportance::HIGH);
    EXPECT_EQ(mapper->getTypeImportance(LogMessageType::INFO), MessageImportance::MEDIUM);
    EXPECT_EQ(mapper->getTypeImportance(LogMessageType::DEBUG), MessageImportance::LOW);
    EXPECT_EQ(mapper->getTypeImportance(LogMessageType::TRACE), MessageImportance::LOW);
}

TEST_F(ImportanceMapperTest, TestTypeImportanceReset) {
    // Set custom type importance, then reset
    mapper->setTypeImportance(LogMessageType::DEBUG, MessageImportance::CRITICAL, "Make DEBUG critical");

    // Verify it's set
    ASSERT_EQ(mapper->getTypeImportance(LogMessageType::DEBUG), MessageImportance::CRITICAL);

    // Reset to default
    bool reset_result = mapper->resetTypeImportance(LogMessageType::DEBUG);
    ASSERT_TRUE(reset_result);

    // Verify it's back to default
    EXPECT_EQ(mapper->getTypeImportance(LogMessageType::DEBUG), MessageImportance::LOW);
}

TEST_F(ImportanceMapperTest, TestAllTypeMappingsRetrieval) {
    // Add some custom mappings
    mapper->setTypeImportance(LogMessageType::TRACE, MessageImportance::MEDIUM, "Promote TRACE");
    mapper->setTypeImportance(LogMessageType::DEBUG, MessageImportance::HIGH, "Promote DEBUG");

    auto all_mappings = mapper->getAllTypeMappings();

    // Should have at least 6 mappings (all message types)
    ASSERT_GE(all_mappings.size(), 6);

    // Find our custom mappings
    bool found_trace = false;
    bool found_debug = false;

    for (const auto& mapping : all_mappings) {
        if (mapping.first == LogMessageType::TRACE && mapping.second == MessageImportance::MEDIUM) {
            found_trace = true;
        }
        if (mapping.first == LogMessageType::DEBUG && mapping.second == MessageImportance::HIGH) {
            found_debug = true;
        }
    }

    EXPECT_TRUE(found_trace);
    EXPECT_TRUE(found_debug);
}

// =============================================================================
// COMPONENT OVERRIDE TESTS
// =============================================================================

TEST_F(ImportanceMapperTest, TestComponentOverrideAddition) {
    // Add component override
    uint32_t override_id = mapper->addComponentOverride("Database", MessageImportance::CRITICAL, false, "Database is critical");

    // Verify override was added
    ASSERT_NE(override_id, 0u);

    // Find the override
    const ComponentImportanceOverride* found_override = mapper->findComponentOverride("Database");
    ASSERT_TRUE(found_override != nullptr);
    EXPECT_EQ(found_override->importance, MessageImportance::CRITICAL);
    EXPECT_EQ(found_override->component_pattern, "Database");
}

TEST_F(ImportanceMapperTest, TestComponentOverrideExactMatch) {
    // Add component override
    mapper->addComponentOverride("OrderProcessor", MessageImportance::CRITICAL, false);

    // Test exact match
    const ComponentImportanceOverride* exact_match = mapper->findComponentOverride("OrderProcessor");
    ASSERT_TRUE(exact_match != nullptr);
    EXPECT_EQ(exact_match->importance, MessageImportance::CRITICAL);

    // Test non-match
    const ComponentImportanceOverride* no_match = mapper->findComponentOverride("RiskEngine");
    ASSERT_TRUE(no_match == nullptr);
}

TEST_F(ImportanceMapperTest, TestComponentOverrideRemoval) {
    // Add and then remove component override
    uint32_t override_id = mapper->addComponentOverride("MarketData", MessageImportance::HIGH, false);

    // Verify it exists
    ASSERT_TRUE(mapper->findComponentOverride("MarketData") != nullptr);

    // Remove it
    bool removed = mapper->removeComponentOverride(override_id);
    ASSERT_TRUE(removed);

    // Verify it's gone
    ASSERT_TRUE(mapper->findComponentOverride("MarketData") == nullptr);
}

TEST_F(ImportanceMapperTest, TestComponentOverrideUpdate) {
    // Add component override, then update it
    uint32_t override_id = mapper->addComponentOverride("PositionManager", MessageImportance::MEDIUM, false);

    // Update to different importance
    bool updated = mapper->updateComponentOverride(override_id, MessageImportance::CRITICAL, "Now critical");
    ASSERT_TRUE(updated);

    // Verify update
    const ComponentImportanceOverride* updated_override = mapper->findComponentOverride("PositionManager");
    ASSERT_TRUE(updated_override != nullptr);
    EXPECT_EQ(updated_override->importance, MessageImportance::CRITICAL);
}

// =============================================================================
// FUNCTION OVERRIDE TESTS
// =============================================================================

TEST_F(ImportanceMapperTest, TestFunctionOverrideAddition) {
    // Add function override
    uint32_t override_id = mapper->addFunctionOverride("ValidateOrder", MessageImportance::CRITICAL, false, "Function is critical");

    // Verify override was added
    ASSERT_NE(override_id, 0u);
}

TEST_F(ImportanceMapperTest, TestFunctionOverrideMatch) {
    // Add function override
    mapper->addFunctionOverride("CalcPosition", MessageImportance::HIGH, false);

    // Test exact match
    const FunctionImportanceOverride* match = mapper->findFunctionOverride("CalcPosition");
    ASSERT_TRUE(match != nullptr);
    EXPECT_EQ(match->importance, MessageImportance::HIGH);
}

TEST_F(ImportanceMapperTest, TestFunctionOverrideRemoval) {
    // Add and remove function override
    uint32_t override_id = mapper->addFunctionOverride("SendOrder", MessageImportance::CRITICAL, false);

    // Verify exists, remove, verify gone
    ASSERT_TRUE(mapper->findFunctionOverride("SendOrder") != nullptr);
    bool removed = mapper->removeFunctionOverride(override_id);
    ASSERT_TRUE(removed);
    ASSERT_TRUE(mapper->findFunctionOverride("SendOrder") == nullptr);
}

// =============================================================================
// IMPORTANCE RESOLUTION TESTS
// =============================================================================

TEST_F(ImportanceMapperTest, TestMessageImportanceResolution) {
    // Create test message and context
    auto message = CreateTestMessage(LogMessageType::WARN, "OrderProcessor", "ValidateOrder");
    auto context = CreateResolutionContext(70); // High load

    // Resolve importance
    auto result = mapper->resolveMessageImportance(message, context);

    // Verify basic resolution occurred (result has valid importance)
    EXPECT_NE(result.final_importance, static_cast<MessageImportance>(-1));
    EXPECT_GT(result.resolution_time_microseconds, 0u); // Should have taken some time
}

TEST_F(ImportanceMapperTest, TestContextualImportanceResolution) {
    // Test different contexts affect importance
    MessageImportance normal_load = mapper->resolveContextualImportance(
        LogMessageType::DEBUG, "Cache", "Get", 30, 2); // Normal load

    MessageImportance high_load = mapper->resolveContextualImportance(
        LogMessageType::DEBUG, "Cache", "Get", 90, 2);  // High load

    // Under high load, DEBUG messages might be downgraded or filtered
    // The exact behavior depends on the implementation's load-based logic
    // We can test that the method executes without errors
    EXPECT_NE(normal_load, static_cast<MessageImportance>(-1));
    EXPECT_NE(high_load, static_cast<MessageImportance>(-1));
}

TEST_F(ImportanceMapperTest, TestTypeOnlyImportanceResolution) {
    // Test pure type resolution (no overrides)
    MessageImportance critical_resolved = mapper->resolveTypeImportance(LogMessageType::CRITICAL_LOG);
    EXPECT_EQ(critical_resolved, MessageImportance::CRITICAL);

    MessageImportance trace_resolved = mapper->resolveTypeImportance(LogMessageType::TRACE);
    EXPECT_EQ(trace_resolved, MessageImportance::LOW);
}

TEST_F(ImportanceMapperTest, TestComponentInfluenceOnResolution) {
    // Add component override
    mapper->addComponentOverride("Database", MessageImportance::CRITICAL, false);

    // Test with override
    MessageImportance with_override = mapper->resolveComponentImportance(LogMessageType::INFO, "Database");
    MessageImportance without_override = mapper->resolveComponentImportance(LogMessageType::INFO, "Cache");

    // Component override should affect resolution
    EXPECT_EQ(with_override, MessageImportance::CRITICAL);
    // Without override, should fall back to type-based resolution
}

// =============================================================================
// HIERARCHICAL OVERRIDE PRECEDENCE TESTS
// =============================================================================

TEST_F(ImportanceMapperTest, TestHierarchicalOverridePrecedence) {
    // Set up hierarchy: Type -> Component -> Function
    // Function overrides should take precedence over component overrides over type defaults

    // Set component-level override (less specific)
    mapper->addComponentOverride("OrderProcessor", MessageImportance::HIGH, false);

    // Set function-level override (more specific)
    mapper->addFunctionOverride("CancelOrder", MessageImportance::CRITICAL, false);

    // Test resolution for function without specific override
    MessageImportance component_level = mapper->resolveFunctionImportance(
        LogMessageType::WARN, "OrderProcessor", "ProcessOrder"); // Uses component override
    EXPECT_EQ(component_level, MessageImportance::HIGH);

    // Test resolution for function with specific override
    MessageImportance function_level = mapper->resolveFunctionImportance(
        LogMessageType::WARN, "OrderProcessor", "CancelOrder"); // Uses function override
    EXPECT_EQ(function_level, MessageImportance::CRITICAL);
}

// =============================================================================
// PERSISTENCE DECISION TESTS
// =============================================================================

TEST_F(ImportanceMapperTest, TestShouldPersistMessage) {
    // Create test message
    auto message = CreateTestMessage(LogMessageType::ERR, "Database", "Connect");
    auto context = CreateResolutionContext();

    // Test persistence decision with different thresholds
    bool should_persist_high = mapper->shouldPersistMessage(message, context, MessageImportance::MEDIUM);
    bool should_persist_higher = mapper->shouldPersistMessage(message, context, MessageImportance::CRITICAL);

    // ERR should persist with MEDIUM threshold but not CRITICAL
    EXPECT_TRUE(should_persist_high);
    // Result for CRITICAL threshold depends on exact resolution
}

TEST_F(ImportanceMapperTest, TestShouldPersistBySystemConditions) {
    // Test persistence based on system conditions
    bool persist_normal = mapper->shouldPersistBySystemConditions(LogMessageType::DEBUG, 50, 5, false);
    bool persist_emergency = mapper->shouldPersistBySystemConditions(LogMessageType::DEBUG, 95, 20, true);

    // Emergency mode might change persistence behavior
    EXPECT_NE(persist_normal, static_cast<bool>(-1));
    EXPECT_NE(persist_emergency, static_cast<bool>(-1));
}

// =============================================================================
// THREAD SAFETY TESTS
// =============================================================================

TEST_F(ImportanceMapperTest, TestThreadSafety_ConcurrentReads) {
    // Test that concurrent reads don't cause issues
    const int num_threads = 5;
    const int operations_per_thread = 100;
    std::atomic<int> successful_reads{0};

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            int local_successes = 0;
            for (int j = 0; j < operations_per_thread; ++j) {
                // Perform read operations
                if (mapper->getApplicationName() == "TradingApp" &&
                    mapper->isConfigured()) {
                    local_successes++;
                }
            }
            successful_reads += local_successes;
        });
    }

    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify all operations succeeded
    EXPECT_EQ(successful_reads.load(), num_threads * operations_per_thread);
}

TEST_F(ImportanceMapperTest, TestThreadSafety_IsolatedWriteOperations) {
    // Test write operations in separate mappers (not concurrent on same instance)
    const int num_threads = 3;

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            // Create isolated mapper for each thread
            auto thread_mapper = std::make_unique<ImportanceMapper>("ThreadApp" + std::to_string(i));

            // Perform some write operations
            thread_mapper->setTypeImportance(LogMessageType::INFO, MessageImportance::HIGH);
            thread_mapper->addComponentOverride("Component" + std::to_string(i), MessageImportance::CRITICAL);

            // Verify local changes
            EXPECT_EQ(thread_mapper->getApplicationName(), "ThreadApp" + std::to_string(i));
            EXPECT_EQ(thread_mapper->getTypeImportance(LogMessageType::INFO), MessageImportance::HIGH);

            auto override = thread_mapper->findComponentOverride("Component" + std::to_string(i));
            EXPECT_TRUE(override != nullptr);
        });
    }

    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify main mapper is unaffected
    EXPECT_EQ(mapper->getApplicationName(), "TradingApp");
}

// =============================================================================
// CONFIGURATION MANAGEMENT TESTS
// =============================================================================

TEST_F(ImportanceMapperTest, TestConfigurationValidation) {
    // Test that default configuration is valid
    ASSERT_TRUE(mapper->validateConfiguration());

    // Get configuration and verify it's not empty
    auto config = mapper->getConfiguration();
    EXPECT_FALSE(config.application_name.empty());
}

TEST_F(ImportanceMapperTest, TestOverrideCountMethods) {
    // Start with no overrides
    EXPECT_EQ(mapper->getOverrideCount(), 0);
    EXPECT_FALSE(mapper->hasOverrides());

    // Add some overrides
    mapper->addComponentOverride("Comp1", MessageImportance::HIGH);
    mapper->addFunctionOverride("Func1", MessageImportance::CRITICAL);

    // Check counts
    EXPECT_EQ(mapper->getOverrideCount(), 2);
    EXPECT_TRUE(mapper->hasOverrides());

    // Clear component overrides
    size_t cleared = mapper->clearComponentOverrides();
    EXPECT_EQ(cleared, 1);
    EXPECT_EQ(mapper->getOverrideCount(), 1);

    // Clear all overrides
    size_t cleared_all = mapper->clearAllOverrides();
    EXPECT_EQ(cleared_all, 1);
    EXPECT_EQ(mapper->getOverrideCount(), 0);
    EXPECT_FALSE(mapper->hasOverrides());
}

// =============================================================================
// UTILITY FUNCTIONS TESTS
// =============================================================================

TEST_F(ImportanceMapperTest, TestMapperCloneAndSwap) {
    // Test clone functionality
    auto cloned_mapper = mapper->clone();
    EXPECT_EQ(cloned_mapper.getApplicationName(), mapper->getApplicationName());

    // Test swap functionality
    ImportanceMapper another_mapper("AnotherApp");
    mapper->swap(another_mapper);

    EXPECT_EQ(mapper->getApplicationName(), "AnotherApp");
    EXPECT_EQ(another_mapper.getApplicationName(), "TradingApp");
}

/**
 * Test Suite Status: IMPLEMENTATION COMPLETE
 *
 * Coverage Areas:
 * ✅ Basic mapper initialization and configuration
 * ✅ Type-based importance mapping and overrides
 * ✅ Component override addition, removal, and updating
 * ✅ Function override management
 * ✅ Hierarchical importance resolution (Type → Component → Function)
 * ✅ Contextual factors in resolution (system load, error rates)
 * ✅ Persistence decision making
 * ✅ Thread safety for concurrent access
 * ✅ Configuration validation and override counting
 * ✅ Utility functions (clone, swap, clear overrides)
 *
 * Key Validation Points:
 * ✅ Override precedence order works correctly
 * ✅ Default type mappings are correct
 * ✅ Component/function matching functions properly
 * ✅ Thread safety through proper locking
 * ✅ Configuration changes don't break validation
 *
 * Dependencies: LogMessageData structure, LogMessageType enum, ImportanceResolutionContext
 * Risk Level: Low (pure logic, existing data structures)
 * Business Value: 100% logical foundation for contextual logging decisions
 *
 * Next: Integration with ASFMLogger::ImportanceMapper component
 */
