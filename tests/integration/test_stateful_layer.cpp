/**
 * ASFMLogger Stateful Layer Compilation Test Suite
 *
 * Tests that stateful layer components compile and can be instantiated.
 * Focus on ensuring no linker errors and basic functionality works.
 */

#include <gtest/gtest.h>
#include <iostream>

// Stateful components
#include "stateful/LogMessage.hpp"
#include "stateful/LoggerInstance.hpp"
#include "stateful/ImportanceMapper.hpp"

TEST(StatefulLayerCompilationTest, BasicInstantiationWorks) {
    std::cout << "Testing basic stateful layer component instantiation..." << std::endl;

    // Test LogMessage creation and basic methods
    auto msg = LogMessage::Create(LogMessageType::INFO, "Test message");
    EXPECT_TRUE(msg.isValid());
    EXPECT_FALSE(msg.empty());
    EXPECT_TRUE(msg.getId() > 0);

    // Test LoggerInstance creation and basic methods
    auto instance = LoggerInstance::Create("TestApp", "TestProcess");
    EXPECT_TRUE(instance.isValid());
    EXPECT_FALSE(instance.empty());
    EXPECT_TRUE(instance.getId() > 0);

    // Test ImportanceMapper creation and basic methods
    auto mapper = ImportanceMapper("TestMapper");
    EXPECT_EQ(mapper.getApplicationName(), "TestMapper");

    std::cout << "All stateful components basic instantiation test PASSED" << std::endl;
    SUCCEED();
}

// =============================================================================
// LOGGERINSTANCE TESTS
// =============================================================================

class LoggerInstanceTest : public Test {
protected:
    void SetUp() override {
        test_instance_ = LoggerInstance::Create("TestApp", "TestProcess", "TestInstance");
    }

    LoggerInstance test_instance_;
};

TEST_F(LoggerInstanceTest, Constructor) {
    EXPECT_TRUE(test_instance_.isValid());
    EXPECT_EQ(test_instance_.getApplicationName(), "TestApp");
    EXPECT_EQ(test_instance_.getProcessName(), "TestProcess");
    EXPECT_EQ(test_instance_.getInstanceName(), "TestInstance");
    EXPECT_TRUE(test_instance_.getId() > 0);
    EXPECT_TRUE(test_instance_.getProcessId() > 0);
    EXPECT_GE(test_instance_.getCreationTime(), 0);
    EXPECT_GE(test_instance_.getLastActivity(), 0);
}

TEST_F(LoggerInstanceTest, Clone) {
    auto cloned = test_instance_.clone();
    EXPECT_EQ(test_instance_.getApplicationName(), cloned.getApplicationName());
    EXPECT_EQ(test_instance_.getProcessName(), cloned.getProcessName());
    EXPECT_EQ(test_instance_.getInstanceName(), cloned.getInstanceName());
    EXPECT_EQ(test_instance_.getId(), cloned.getId());
}

TEST_F(LoggerInstanceTest, Setters) {
    EXPECT_TRUE(test_instance_.setApplicationInfo("UpdatedApp", "UpdatedProcess"));
    EXPECT_EQ(test_instance_.getApplicationName(), "UpdatedApp");
    EXPECT_EQ(test_instance_.getProcessName(), "UpdatedProcess");

    EXPECT_TRUE(test_instance_.setInstanceName("UpdatedInstance"));
    EXPECT_EQ(test_instance_.getInstanceName(), "UpdatedInstance");
}

TEST_F(LoggerInstanceTest, ActivityTracking) {
    uint64_t initial_messages = test_instance_.getMessageCount();
    uint64_t initial_errors = test_instance_.getErrorCount();

    test_instance_.incrementMessageCount();
    test_instance_.incrementErrorCount();

    EXPECT_EQ(test_instance_.getMessageCount(), initial_messages + 1);
    EXPECT_EQ(test_instance_.getErrorCount(), initial_errors + 1);

    test_instance_.updateActivity();
    // Activity timestamp should be updated
    EXPECT_GE(test_instance_.getLastActivity(), 0);
}

TEST_F(LoggerInstanceTest, Validation) {
    EXPECT_TRUE(test_instance_.isValid());
    EXPECT_TRUE(test_instance_.hasApplicationInfo());
    EXPECT_TRUE(test_instance_.isActive()); // Recently created
    EXPECT_FALSE(test_instance_.empty());
    EXPECT_GE(test_instance_.getUptime(), 0);
    EXPECT_GE(test_instance_.getIdleTime(), 0);
}

TEST_F(LoggerInstanceTest, Statistics) {
    double message_rate = test_instance_.getMessageRate();
    double error_rate = test_instance_.getErrorRate();

    EXPECT_GE(message_rate, 0.0);
    EXPECT_GE(error_rate, 0.0);
}

TEST_F(LoggerInstanceTest, Comparison) {
    auto old_instance = LoggerInstance::Create("OldApp", "OldProcess", "Old");
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Ensure different timestamps
    auto new_instance = LoggerInstance::Create("NewApp", "NewProcess", "New");

    EXPECT_TRUE(old_instance < new_instance);
    EXPECT_TRUE(new_instance > old_instance);
    EXPECT_FALSE(old_instance > new_instance);
    EXPECT_FALSE(new_instance < old_instance);

    EXPECT_TRUE(old_instance == old_instance);
    EXPECT_FALSE(old_instance == new_instance);
}

TEST_F(LoggerInstanceTest, ApplicationComparison) {
    auto app1_instance1 = LoggerInstance::Create("TestApp", "Process1", "Inst1");
    auto app1_instance2 = LoggerInstance::Create("TestApp", "Process2", "Inst2");
    auto app2_instance1 = LoggerInstance::Create("OtherApp", "Process1", "Inst1");

    EXPECT_TRUE(app1_instance1.isSameApplication(app1_instance2));
    EXPECT_FALSE(app1_instance1.isSameApplication(app2_instance1));
    EXPECT_TRUE(app1_instance1.isSameProcess(app1_instance1));
    EXPECT_FALSE(app1_instance1.isSameProcess(app1_instance2));
}

TEST_F(LoggerInstanceTest, Swap) {
    auto instance1 = LoggerInstance::Create("App1", "Proc1", "Inst1");
    auto instance2 = LoggerInstance::Create("App2", "Proc2", "Inst2");

    std::string app1 = instance1.getApplicationName();
    std::string app2 = instance2.getApplicationName();

    instance1.swap(instance2);

    EXPECT_EQ(instance1.getApplicationName(), app2);
    EXPECT_EQ(instance2.getApplicationName(), app1);
}

TEST_F(LoggerInstanceTest, Clear) {
    test_instance_.clear();
    EXPECT_TRUE(test_instance_.isValid()); // Should be valid with defaults
    EXPECT_STREQ(test_instance_.getApplicationName().c_str(), "DefaultApplication");
}

TEST_F(LoggerInstanceTest, TwoInstancesDifferentIds) {
    auto instance1 = LoggerInstance::Create("App", "Proc", "Inst");
    auto instance2 = LoggerInstance::Create("App", "Proc", "Inst");

    EXPECT_NE(instance1.getId(), instance2.getId());
}

TEST_F(LoggerInstanceTest, Formatting) {
    std::string string_repr = test_instance_.toString();
    EXPECT_FALSE(string_repr.empty());
    EXPECT_NE(string_repr.find("TestApp"), std::string::npos);

    std::string json_repr = test_instance_.toJson();
    EXPECT_FALSE(json_repr.empty());
    EXPECT_NE(json_repr.find("TestApp"), std::string::npos);

    std::string csv_repr = test_instance_.toCsv();
    EXPECT_FALSE(csv_repr.empty());

    std::string console_repr = test_instance_.formatForConsole();
    EXPECT_FALSE(console_repr.empty());
}

// =============================================================================
// IMPORTANCEMAPPER TESTS
// =============================================================================

class ImportanceMapperTest : public Test {
protected:
    void SetUp() override {
        mapper_ = std::make_unique<ImportanceMapper>("TestApplication");
    }

    ImportanceMapper& getMapper() { return *mapper_; }

    std::unique_ptr<ImportanceMapper> mapper_;
};

TEST_F(ImportanceMapperTest, Constructor) {
    auto& mapper = getMapper();
    EXPECT_STREQ(mapper.getApplicationName().c_str(), "TestApplication");
}

TEST_F(ImportanceMapperTest, TypeImportanceMapping) {
    auto& mapper = getMapper();

    EXPECT_TRUE(mapper.setTypeImportance(LogMessageType::ERROR, MessageImportance::CRITICAL));
    EXPECT_EQ(mapper.getTypeImportance(LogMessageType::ERROR), MessageImportance::CRITICAL);

    EXPECT_TRUE(mapper.resetTypeImportance(LogMessageType::ERROR));
    EXPECT_EQ(mapper.getTypeImportance(LogMessageType::ERROR), MessageImportance::HIGH); // Default for ERROR
}

TEST_F(ImportanceMapperTest, ComponentOverrides) {
    auto& mapper = getMapper();

    uint32_t id = mapper.addComponentOverride("Critical*", MessageImportance::CRITICAL, true);
    EXPECT_GT(id, 0);

    auto overrides = mapper.getComponentOverrides();
    EXPECT_EQ(overrides.size(), 1);
    EXPECT_STREQ(overrides[0].pattern.c_str(), "Critical*");

    EXPECT_TRUE(mapper.removeComponentOverride(id));
    EXPECT_EQ(mapper.getComponentOverrides().size(), 0);
}

TEST_F(ImportanceMapperTest, FunctionOverrides) {
    auto& mapper = getMapper();

    uint32_t id = mapper.addFunctionOverride("validate*", MessageImportance::HIGH, true);
    EXPECT_GT(id, 0);

    auto overrides = mapper.getFunctionOverrides();
    EXPECT_EQ(overrides.size(), 1);

    EXPECT_TRUE(mapper.removeFunctionOverride(id));
    EXPECT_EQ(mapper.getFunctionOverrides().size(), 0);
}

TEST_F(ImportanceMapperTest, ImportanceResolution) {
    auto& mapper = getMapper();

    // Set up overrides
    mapper.addComponentOverride("Database", MessageImportance::HIGH);
    mapper.addFunctionOverride("connect*", MessageImportance::CRITICAL);

    // Create test messages
    auto base_msg = LogMessage::Create(LogMessageType::WARN, "Test", "Database", "connectDB");
    auto normal_msg = LogMessage::Create(LogMessageType::WARN, "Test", "Normal", "normalFunc");

    // Resolve importance
    auto base_resolution = mapper.resolveMessageImportance(base_msg, {});
    auto normal_resolution = mapper.resolveMessageImportance(normal_msg, {});

    // Component override should make database messages higher importance
    EXPECT_EQ(base_resolution.resolved_importance, MessageImportance::CRITICAL); // Function override wins
    EXPECT_EQ(normal_resolution.resolved_importance, MessageImportance::HIGH);  // Type-based
}

TEST_F(ImportanceMapperTest, Statistics) {
    auto& mapper = getMapper();

    // Add some overrides
    mapper.addComponentOverride("Comp*", MessageImportance::HIGH, true);
    mapper.addFunctionOverride("Func*", MessageImportance::CRITICAL, true);

    auto stats = mapper.getStatistics();
    EXPECT_GT(stats.override_count, 0);
    EXPECT_GT(stats.last_activity_timestamp, 0);
}

TEST_F(ImportanceMapperTest, ConfigurationReset) {
    auto& mapper = getMapper();

    // Add overrides
    mapper.addComponentOverride("Test*", MessageImportance::HIGH);
    mapper.addFunctionOverride("test*", MessageImportance::CRITICAL);

    EXPECT_TRUE(mapper.hasOverrides());
    EXPECT_GT(mapper.getOverrideCount(), 0);

    // Clear all
    size_t cleared = mapper.clearAllOverrides();
    EXPECT_EQ(cleared, 2);
    EXPECT_EQ(mapper.getOverrideCount(), 0);
}

TEST_F(ImportanceMapperTest, Clone) {
    auto& mapper = getMapper();

    // Set up some configuration
    mapper.addComponentOverride("Clone*", MessageImportance::HIGH);

    auto cloned = mapper.clone();

    EXPECT_STREQ(cloned.getApplicationName().c_str(), mapper.getApplicationName().c_str());
    EXPECT_EQ(cloned.getOverrideCount(), mapper.getOverrideCount());
}

TEST_F(ImportanceMapperTest, Swap) {
    ImportanceMapper mapper1("App1");
    ImportanceMapper mapper2("App2");

    // Add different overrides
    mapper1.addComponentOverride("App1*", MessageImportance::HIGH);
    mapper2.addComponentOverride("App2*", MessageImportance::CRITICAL);

    std::string app1 = mapper1.getApplicationName();
    std::string app2 = mapper2.getApplicationName();

    mapper1.swap(mapper2);

    EXPECT_EQ(mapper1.getApplicationName(), app2);
    EXPECT_EQ(mapper2.getApplicationName(), app1);
    EXPECT_EQ(mapper2.getOverrideCount(), 1); // Had one override
    EXPECT_EQ(mapper1.getOverrideCount(), 1); // Had one override
}

// =============================================================================
// INTEGRATION TESTS
// =============================================================================

class StatefulLayerIntegrationTest : public Test {
protected:
    void SetUp() override {
        instance_ = LoggerInstance::Create("IntegrationTest", "TestProcess");
        mapper_ = std::make_unique<ImportanceMapper>("IntegrationTest");
        message_ = LogMessage::Create(LogMessageType::INFO, "Integration test message",
                                    "TestComponent", "testFunction", "integration.cpp", 123);
    }

    LoggerInstance instance_;
    std::unique_ptr<ImportanceMapper> mapper_;
    LogMessage message_;
};

TEST_F(StatefulLayerIntegrationTest, MessageWithInstance) {
    // Verify message has valid ID
    EXPECT_TRUE(message_.getId() > 0);
    EXPECT_TRUE(message_.getInstanceId() > 0);

    // Instance also has valid ID
    EXPECT_TRUE(instance_.getId() > 0);
    EXPECT_TRUE(instance_.getProcessId() > 0);
}

TEST_F(StatefulLayerIntegrationTest, ImportanceWithMessage) {
    // Configure importance for our component
    mapper_->addComponentOverride("TestComponent", MessageImportance::HIGH);

    // Resolve importance for our message
    auto resolution = mapper_->resolveMessageImportance(message_, {});

    // Should be HIGH due to component override
    EXPECT_EQ(resolution.resolved_importance, MessageImportance::HIGH);
    EXPECT_GE(resolution.rule_evaluation_time_usec, 0);
}

TEST_F(StatefulLayerIntegrationTest, InstanceActivityTracking) {
    // Simulate some activity
    for (int i = 0; i < 5; ++i) {
        instance_.incrementMessageCount();
    }

    EXPECT_EQ(instance_.getMessageCount(), 5);
    EXPECT_GE(instance_.getMessageRate(), 0.0);
}

TEST_F(StatefulLayerIntegrationTest, CrossComponentDataFlow) {
    // Create message
    auto test_message = LogMessage::Create(LogMessageType::WARN, "Cross-component test",
                                         "FlowComp", "testFlow", "flow.cpp", 456);

    // Configure importance
    mapper_->setTypeImportance(LogMessageType::WARN, MessageImportance::MEDIUM);

    // Check resolution
    auto resolution = mapper_->resolveMessageImportance(test_message, {});

    EXPECT_EQ(resolution.resolved_importance, MessageImportance::MEDIUM);

    // Format message for different outputs
    std::string json = test_message.toJson();
    std::string csv = test_message.toCsv();
    std::string console = test_message.formatForConsole();

    EXPECT_FALSE(json.empty());
    EXPECT_FALSE(csv.empty());
    EXPECT_FALSE(console.empty());

    EXPECT_NE(json.find("Cross-component test"), std::string::npos);
    EXPECT_NE(console.find("FlowComp"), std::string::npos);
}

// =============================================================================
// THREAD SAFETY TESTS
// =============================================================================

class StatefulLayerThreadSafetyTest : public Test {
protected:
    void SetUp() override {
        instance_ = LoggerInstance::Create("ThreadTest", "TestProcess");
        mapper_ = std::make_unique<ImportanceMapper>("ThreadTest");
    }

    const int NUM_THREADS = 5;
    const int OPERATIONS_PER_THREAD = 100;

    void RunInMultipleThreads(std::function<void()> operation) {
        std::vector<std::thread> threads;

        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back(operation);
        }

        for (auto& thread : threads) {
            thread.join();
        }
    }

    LoggerInstance instance_;
    std::unique_ptr<ImportanceMapper> mapper_;
    std::atomic<int> shared_counter_{0};
};

TEST_F(StatefulLayerThreadSafetyTest, LoggerInstanceActivityThreadSafety) {
    RunInMultipleThreads([this]() {
        for (int i = 0; i < OPERATIONS_PER_THREAD; ++i) {
            this->instance_.incrementMessageCount();
            this->instance_.updateActivity();
        }
    });

    EXPECT_EQ(instance_.getMessageCount(), NUM_THREADS * OPERATIONS_PER_THREAD);
}

TEST_F(StatefulLayerThreadSafetyTest, ImportanceMapperThreadSafety) {
    std::vector<uint32_t> override_ids;

    // Add overrides from multiple threads
    RunInMultipleThreads([this]() {
        for (int i = 0; i < OPERATIONS_PER_THREAD; ++i) {
            auto id = this->mapper_->addComponentOverride("ThreadComponent" + std::to_string(i),
                                                        MessageImportance::MEDIUM);
            override_ids.push_back(id);
        }
    });

    // Verify all overrides were added
    auto overrides = mapper_->getComponentOverrides();
    EXPECT_EQ(overrides.size(), NUM_THREADS * OPERATIONS_PER_THREAD);

    // Remove some overrides
    RunInMultipleThreads([this, &override_ids]() {
        for (size_t i = 0; i < override_ids.size(); ++i) {
            if (i % 2 == 0) {
                this->mapper_->removeComponentOverride(override_ids[i]);
            }
        }
    });

    // Should have roughly half the overrides remaining (some statistical variation)
    auto remaining_overrides = mapper_->getComponentOverrides();
    EXPECT_LT(remaining_overrides.size(), overrides.size());
}

// =============================================================================
// PERFORMANCE TESTS
// =============================================================================

class StatefulLayerPerformanceTest : public Test {
protected:
    const int PERFORMANCE_TEST_ITERATIONS = 10000;

    void SetUp() override {
        instance_ = LoggerInstance::Create("PerfTest", "TestProcess");
        mapper_ = std::make_unique<ImportanceMapper>("PerfTest");
        message_ = LogMessage::Create(LogMessageType::INFO, "Performance test message",
                                    "PerfComponent", "perfFunction");
    }

    LoggerInstance instance_;
    std::unique_ptr<ImportanceMapper> mapper_;
    LogMessage message_;
};

TEST_F(StatefulLayerPerformanceTest, LogMessageOperations) {
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < PERFORMANCE_TEST_ITERATIONS; ++i) {
        auto msg = LogMessage::Create(LogMessageType::INFO, "Test message " + std::to_string(i),
                                    "Component", "Function");
        msg.setMessage("Updated message " + std::to_string(i));
        std::string formatted = msg.toString();
        (void)formatted; // Avoid unused variable warning
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start).count();

    // Should be under reasonable time (adjust threshold based on testing)
    EXPECT_LT(duration, 5000.0); // 5 seconds total
}

TEST_F(StatefulLayerPerformanceTest, LoggerInstanceOperations) {
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < PERFORMANCE_TEST_ITERATIONS; ++i) {
        instance_.incrementMessageCount();
        instance_.updateActivity();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start).count();

    EXPECT_LT(duration, 2000.0); // 2 seconds total
}

TEST_F(StatefulLayerPerformanceTest, ImportanceMapperOperations) {
    // Set up some configuration
    mapper_->addComponentOverride("Perf*", MessageImportance::HIGH, true);
    mapper_->addFunctionOverride("perf*", MessageImportance::CRITICAL, true);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < PERFORMANCE_TEST_ITERATIONS; ++i) {
        auto resolution = mapper_->resolveMessageImportance(message_, {});
        (void)resolution; // Avoid unused variable warning
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start).count();

    EXPECT_LT(duration, 3000.0); // 3 seconds total
}

// =============================================================================
// COMPILATION TEST - JUST ENSURE EVERYTHING COMPILES
// =============================================================================

TEST(StatefulLayerCompilationTest, AllComponentsCompile) {
    // Test that all stateful components can be instantiated
    auto msg = LogMessage::Create(LogMessageType::INFO, "Compilation test");
    auto instance = LoggerInstance::Create("CompileTest", "TestProcess");
    auto mapper = ImportanceMapper("CompileMapper");

    // Test basic operations compile
    msg.isValid();
    instance.isValid();
    mapper.getApplicationName();

    SUCCEED(); // If we get here, compilation was successful
}
