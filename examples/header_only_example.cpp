/**
 * ASFMLogger Header-Only Usage Example
 *
 * This example demonstrates how to use ASFMLogger in header-only mode.
 * No building or linking required - just include and use!
 */

#define ASFMLOGGER_HEADER_ONLY
#include "../ASFMLogger.hpp"

#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "ASFMLogger Header-Only Example" << std::endl;
    std::cout << "=============================" << std::endl;

    // Get logger instance - no building required!
    auto logger = Logger::getInstance("HeaderOnlyExample", "MainProcess");

    // Configure logging
    logger->configure(
        true,                           // console_output
        "header_only_example.log",      // log_file_name
        10485760,                       // max_file_size (10MB)
        5,                              // max_files
        spdlog::level::info             // log_level
    );

    std::cout << "Logger configured successfully!" << std::endl;

    // Basic logging
    logger->info("Header-only example application started");
    logger->debug("Debug information: processing user input");
    logger->info("Processing data...");

    // Simulate some work
    for (int i = 0; i < 5; ++i) {
        logger->info("Processing item {} of {}", i + 1, 5);

        // Simulate processing time
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Log progress
        if (i == 2) {
            logger->warn("Processing item 3 - this is taking longer than expected");
        }
    }

    // Error simulation
    logger->error("Simulated error for demonstration: database connection failed");
    logger->info("Error handled gracefully, continuing execution");

    // More logging
    logger->info("Data processing completed successfully");
    logger->info("Generating final report...");

    // Critical operation
    logger->critical("Finalizing all operations - this is a critical step");

    std::cout << "All logging operations completed!" << std::endl;
    std::cout << "Check 'header_only_example.log' for the log file output." << std::endl;

    return 0;
}