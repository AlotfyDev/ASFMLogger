/**
 * ASFMLogger Header-Only Implementation
 *
 * Lightweight, header-only version of ASFMLogger for easy integration.
 * Define ASFMLOGGER_HEADER_ONLY before including ASFMLogger.hpp to use this.
 */

#ifndef __ASFMLOGGER_HEADER_ONLY_HPP__
#define __ASFMLOGGER_HEADER_ONLY_HPP__

#include <iostream>
#include <sstream>
#include <memory>
#include <mutex>
#include <atomic>
#include <string>
#include <cstring>
#include <cstdint>
#include <ctime>
#include <iomanip>

// spdlog header-only includes
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/common.h>

// Simple header-only Logger implementation
class Logger {
public:
    // Delete copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Get the singleton instance
    static std::shared_ptr<Logger> getInstance(const std::string& application_name = "ASFMLogger",
                                               const std::string& process_name = "");

    // Basic logging methods
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

    // Configuration
    void configure(bool console_output = true, const std::string& log_file_name = "app.log",
                   size_t max_file_size = 10485760, size_t max_files = 5,
                   spdlog::level::level_enum log_level = spdlog::level::info);

    bool isConfigured() const { return is_configured_; }

    // Error handling
    static std::string GetLastError();
    static void Clear();

private:
    // Private constructor
    Logger();
    Logger(const std::string& application_name, const std::string& process_name);
    ~Logger();

    // Error storage
    template<typename... Args>
    void storeLastError(const std::string& fmt, Args&&... args);

    // Members
    std::shared_ptr<spdlog::logger> logger_;
    bool is_configured_ = false;
    static std::string last_error_;
    static std::mutex error_mutex_;
};

// Static member definitions
std::string Logger::last_error_;
std::mutex Logger::error_mutex_;

// Header-only implementation
inline std::shared_ptr<Logger> Logger::getInstance(const std::string& application_name,
                                                   const std::string& process_name) {
    static std::shared_ptr<Logger> instance = std::make_shared<Logger>(application_name, process_name);
    return instance;
}

inline Logger::Logger() : Logger("ASFMLogger", "") {}

inline Logger::Logger(const std::string& application_name, const std::string& process_name) {
    try {
        logger_ = spdlog::stdout_color_mt("asfmlogger_default");
        logger_->set_level(spdlog::level::info);
        logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [%t] %v");
        is_configured_ = true;
    } catch (const spdlog::spdlog_ex& ex) {
        logger_ = nullptr;
        is_configured_ = false;
    }
}

inline Logger::~Logger() {
    if (logger_) {
        spdlog::drop(logger_->name());
    }
}

inline void Logger::configure(bool console_output, const std::string& log_file_name,
                              size_t max_file_size, size_t max_files,
                              spdlog::level::level_enum log_level) {
    if (is_configured_) {
        if (logger_) {
            logger_->warn("Logger is already configured. Re-configuration ignored.");
        }
        return;
    }

    try {
        auto dist_sink = std::make_shared<spdlog::sinks::dist_sink_mt>();

        if (console_output) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(log_level);
            dist_sink->add_sink(console_sink);
        }

        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_file_name, max_file_size, max_files);
        file_sink->set_level(log_level);
        dist_sink->add_sink(file_sink);

        logger_ = std::make_shared<spdlog::logger>("asfmlogger", dist_sink);
        logger_->set_level(log_level);
        logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [%t] %v");

        is_configured_ = true;
        if (logger_) {
            logger_->info("ASFMLogger configured successfully.");
        }
    } catch (const spdlog::spdlog_ex& ex) {
        if (logger_) {
            logger_->error("Failed to configure logger: {}", ex.what());
        }
        throw std::runtime_error("Logger configuration failed: " + std::string(ex.what()));
    }
}

template<typename... Args>
inline void Logger::storeLastError(const std::string& fmt, Args&&... args) {
    try {
        std::ostringstream oss;
        oss << fmt;
        int dummy[] = { 0, ((void)(oss << " " << args), 0)... };
        (void)dummy;
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

inline std::string Logger::GetLastError() {
    std::lock_guard<std::mutex> lock(error_mutex_);
    return last_error_;
}

inline void Logger::Clear() {
    std::lock_guard<std::mutex> lock(error_mutex_);
    last_error_.clear();
}

// Explicit template instantiations
template void Logger::trace(const std::string& fmt);
template void Logger::debug(const std::string& fmt);
template void Logger::info(const std::string& fmt);
template void Logger::warn(const std::string& fmt);
template void Logger::error(const std::string& fmt);
template void Logger::critical(const std::string& fmt);

#endif // __ASFMLOGGER_HEADER_ONLY_HPP__