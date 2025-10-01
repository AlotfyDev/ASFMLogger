
// FrameworkLogger.cpp
#include "ASFMLogger.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/common.h>
#include <memory>
#include <vector>
#include <stdexcept>
#include <sstream>



std::string Logger::last_error_; // Define static member
std::mutex Logger::error_mutex_; // Define static mutex

std::shared_ptr<Logger> Logger::getInstance() {
    // C++11 static local variable initialization is thread-safe
    static std::shared_ptr<Logger> instance = std::make_shared<Logger>();
    return instance;
}





Logger::Logger() {
    try {
        // Create a default console logger as a fallback
        logger_ = spdlog::stdout_color_mt("framework_default");
        logger_->set_level(spdlog::level::info);
        logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [%t] %v");
        is_configured_ = true;
    } catch (const spdlog::spdlog_ex& ex) {
        // If the default logger fails, leave logger_ as nullptr
        logger_ = nullptr;
        is_configured_ = false;
    }
}

Logger::~Logger() {
    // Ensure logger is dropped to clean up resources
    if (logger_) {
        spdlog::drop(logger_->name());
    }
}

void Logger::configure(
    bool console_output,
    const std::string& log_file_name,
    size_t max_file_size,
    size_t max_files,
    spdlog::level::level_enum log_level) {
    // Handle default argument for log_level
    if (log_level == static_cast<spdlog::level::level_enum>(1)) {
        log_level = spdlog::level::info;
    }
    if (is_configured_) {
        if (logger_) {
            logger_->warn("Logger is already configured. Re-configuration ignored.");
        }
        return;
    }

    try {
        // Create a distributed sink to combine multiple sinks
        auto dist_sink = std::make_shared<spdlog::sinks::dist_sink_mt>();

        // Add console sink if requested
        if (console_output) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(log_level);
            dist_sink->add_sink(console_sink);
        }

        // Add rotating file sink
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_file_name, max_file_size, max_files);
        file_sink->set_level(log_level);
        dist_sink->add_sink(file_sink);

        // Create the logger with the combined sink
        logger_ = std::make_shared<spdlog::logger>("framework", dist_sink);
        logger_->set_level(log_level);
        logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [%t] %v");

        is_configured_ = true;
        logger_->info("Logger configured successfully.");
    } catch (const spdlog::spdlog_ex& ex) {
        if (logger_) {
            logger_->error("Failed to configure logger: {}", ex.what());
        }
        throw std::runtime_error("Logger configuration failed: " + std::string(ex.what()));
    }
}

void Logger::LogError(const std::string& component, const std::string& function, const std::string& message) {
    std::ostringstream oss;
    oss << "[" << component << "] " << function << ": " << message;
    error(oss.str());
}

template<typename... Args>
void Logger::storeLastError(const std::string& fmt, Args&&... args) {
    try {
        std::ostringstream oss;
        oss << fmt;
        // Simple approach - just append all args separated by spaces
        int dummy[] = { 0, ((void)(oss << " " << args), 0)... };
        (void)dummy; // suppress unused variable warning
        std::string formatted_msg = oss.str();
        {
            std::lock_guard<std::mutex> lock(error_mutex_);
            last_error_ = formatted_msg;
        }
    } catch (const std::exception& ex) {
        std::lock_guard<std::mutex> lock(error_mutex_);
        last_error_ = "Failed to format error message: " + std::string(ex.what());
    } catch (...) {
        std::lock_guard<std::mutex> lock(error_mutex_);
        last_error_ = "Unknown error occurred while formatting error message";
    }
}

std::string Logger::GetLastError() {
    std::lock_guard<std::mutex> lock(error_mutex_);
    return last_error_;
}

void Logger::Clear() {
    std::lock_guard<std::mutex> lock(error_mutex_);
    last_error_.clear();
}


