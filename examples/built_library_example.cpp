/**
 * ASFMLogger Built Library Usage Example
 *
 * This example demonstrates how to use ASFMLogger with the full built library
 * including all enterprise features like multi-instance management,
 * importance framework, and database integration.
 */

#include "ASFMLogger.hpp"

#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "ASFMLogger Built Library Example" << std::endl;
    std::cout << "================================" << std::endl;

    // Get enhanced logger instance with application tracking
    auto logger = Logger::getInstance("BuiltLibraryExample", "MainProcess");

    // Configure with advanced features
    logger->configureEnhanced(
        "BuiltLibraryExample",
        false,  // No database for this example
        "",
        false,  // No shared memory for this example
        "",
        true,   // Console output
        "built_library_example.log",
        10485760,  // 10MB
        5,         // 5 files
        spdlog::level::info
    );

    std::cout << "Enhanced logger configured successfully!" << std::endl;

    // Use enhanced logging with component tracking
    logger->info("Application", "main", "Built library example application started");
    logger->debug("Database", "Connect", "Attempting to connect to database");
    logger->info("Cache", "Init", "Initializing cache system");

    // Simulate some work with different components
    for (int i = 0; i < 3; ++i) {
        logger->info("Processor", "ProcessData", "Processing data batch {} of {}", i + 1, 3);

        // Simulate processing time
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // Log progress with different components
        if (i == 1) {
            logger->warn("Security", "Validate", "Suspicious activity detected during processing");
        }
    }

    // Simulate an error condition
    logger->error("Database", "Query", "Database query failed - connection timeout");
    logger->info("Recovery", "HandleError", "Attempting to recover from database error");

    // Simulate recovery
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    logger->info("Database", "Reconnect", "Database connection restored successfully");

    // More logging with different components
    logger->info("API", "Request", "Processing API request from client");
    logger->info("Cache", "Hit", "Cache hit for user profile data");
    logger->warn("RateLimit", "Check", "Approaching rate limit threshold");

    // Critical operation
    logger->critical("Deployment", "Finalize", "Finalizing deployment - critical operation");

    std::cout << "All logging operations completed!" << std::endl;

    // Get comprehensive statistics
    std::string stats = logger->getComprehensiveStatistics();
    std::cout << "\n=== Logging Statistics ===" << std::endl;
    std::cout << stats << std::endl;

    // Get status
    std::string status = logger->getStatus();
    std::cout << "\n=== Logger Status ===" << std::endl;
    std::cout << status << std::endl;

    std::cout << "\nCheck 'built_library_example.log' for the complete log file output." << std::endl;

    return 0;
}