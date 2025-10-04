/**
 * ASFMLogger Simple Stateful Layer Test
 *
 * Basic compilation test to verify Stateful Layer components work.
 */

#include <gtest/gtest.h>
#include <iostream>
#include "stateful/LogMessage.hpp"
#include "stateful/LoggerInstance.hpp"
#include "stateful/ImportanceMapper.hpp"

// Simple compilation test
TEST(StatefulLayer_Simple, CompilationTest) {
    // Test basic instantiation works
    EXPECT_TRUE(true);

    // Test LogMessage can be created
    auto msg = LogMessage::Create(LogMessageType::INFO, "Test");
    std::cout << "LogMessage created successfully" << std::endl;

    // Test LoggerInstance can be created
    auto instance = LoggerInstance::Create("App", "Process");
    std::cout << "LoggerInstance created successfully" << std::endl;

    // Test ImportanceMapper can be created
    auto mapper = ImportanceMapper("TestApp");
    std::cout << "ImportanceMapper created successfully" << std::endl;

    SUCCEED();
}

TEST(StatefulLayer_Simple, BasicOperations) {
    // Test basic operations exist and compile
    auto msg = LogMessage::Create(LogMessageType::INFO, "Test");
    auto status = msg.isValid();
    EXPECT_TRUE(status);

    auto instance = LoggerInstance::Create("App", "Process");
    auto valid_instance = instance.isValid();
    EXPECT_TRUE(valid_instance);

    auto mapper = ImportanceMapper("App");
    auto app_name = mapper.getApplicationName();
    EXPECT_FALSE(app_name.empty());

    SUCCEED();
}
