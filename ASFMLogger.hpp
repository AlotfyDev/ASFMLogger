#ifndef __ASFMLOGGER_HPP__
#define __ASFMLOGGER_HPP__




// Include spdlog headers or use forward declarations if spdlog is linked as a separate library
// For now, removing direct includes to prevent compilation issues if paths are not correctly configured in build system.
// #include "spdlog/spdlog.h"
// #include "spdlog/sinks/stdout_color_sinks.h"
// #include "spdlog/sinks/rotating_file_sink.h"
// #include "spdlog/sinks/dist_sink.h" // To combine multiple sinks

// Forward declarations for spdlog types to avoid compilation issues when full definitions are not available
#pragma once



// Include spdlog headers - required for proper compilation
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/common.h>



class Logger {
public:
    // Delete copy constructor and assignment operator to enforce singleton
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Get the singleton instance
    static std::shared_ptr<Logger> getInstance();

    // Get the underlying spdlog logger
    std::shared_ptr<spdlog::logger> getLogger() { return logger_; }

    // Convenience logging methods
    template<typename... Args>
    void trace(const std::string& fmt, Args&&... args) {
        if (logger_) logger_->trace(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void debug(const std::string& fmt, Args&&... args) {
        if (logger_) logger_->debug(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void info(const std::string& fmt, Args&&... args) {
        if (logger_) logger_->info(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warn(const std::string& fmt, Args&&... args) {
        if (logger_) logger_->warn(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(const std::string& fmt, Args&&... args) {
        if (logger_) {
            logger_->error(fmt, std::forward<Args>(args)...);
            storeLastError(fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    void critical(const std::string& fmt, Args&&... args) {
        if (logger_) {
            logger_->critical(fmt, std::forward<Args>(args)...);
            storeLastError(fmt, std::forward<Args>(args)...);
        }
    }

    // Specific method for logging errors (used in filemap_agent_mql5_configuration.cpp)
    void LogError(const std::string& component, const std::string& function, const std::string& message);

    // Get the last error message
    static std::string GetLastError();

    // Clear all logged errors
    static void Clear();

    // Configure the logger
    void configure(
        bool console_output = true,
        const std::string& log_file_name = "framework.log",
        size_t max_file_size = 10485760, // 10 MB
        size_t max_files = 5,
        spdlog::level::level_enum log_level = static_cast<spdlog::level::level_enum>(1)
    );

    // Check if logger is configured
    bool isConfigured() const { return is_configured_; }

public:
    // Public constructor and destructor for std::make_shared
    Logger();
    ~Logger();

    // Store the last error message
    template<typename... Args>
    void storeLastError(const std::string& fmt, Args&&... args);

    std::shared_ptr<spdlog::logger> logger_;
    bool is_configured_ = false;
    static std::string last_error_; // Store last error message
    static std::mutex error_mutex_; // Mutex for thread-safe access to last_error_
};











#endif // __ASFMLOGGER_H__