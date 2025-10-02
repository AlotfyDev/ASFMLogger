/**
 * ASFMLogger Header-Only Test
 *
 * Simple test to verify header-only functionality works correctly.
 */

#define ASFMLOGGER_HEADER_ONLY
#include "../ASFMLogger.hpp"

#include <iostream>
#include <cassert>

int main() {
    std::cout << "Testing ASFMLogger Header-Only Implementation..." << std::endl;

    try {
        // Test 1: Get logger instance
        auto logger = Logger::getInstance("TestApp", "TestProcess");
        assert(logger != nullptr);
        std::cout << "✓ Logger instance created successfully" << std::endl;

        // Test 2: Configure logger
        logger->configure(true, "test_header_only.log", 10485760, 5, spdlog::level::info);
        assert(logger->isConfigured());
        std::cout << "✓ Logger configured successfully" << std::endl;

        // Test 3: Basic logging
        logger->info("Header-only test message");
        logger->warn("Header-only warning message");
        logger->error("Header-only error message");
        std::cout << "✓ Basic logging works" << std::endl;

        // Test 4: Error handling
        std::string last_error = Logger::GetLastError();
        assert(!last_error.empty());
        std::cout << "✓ Error handling works" << std::endl;

        // Test 5: Error clearing
        Logger::Clear();
        last_error = Logger::GetLastError();
        assert(last_error.empty());
        std::cout << "✓ Error clearing works" << std::endl;

        std::cout << "🎉 All header-only tests passed!" << std::endl;
        std::cout << "📝 Check 'test_header_only.log' for log file output" << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}