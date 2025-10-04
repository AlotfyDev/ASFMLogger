/**
 * ASFMLogger Logging Toolbox Implementation
 *
 * Static methods for robust logging operations following toolbox architecture.
 * Pure functions for thread-safe logging, file management, and output formatting.
 */

#include "LoggingToolbox.hpp"
#include "TimestampToolbox.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <Windows.h>

// Static variables
LoggingConfiguration LoggingToolbox::global_config_;
std::ofstream LoggingToolbox::log_file_stream_;
std::vector<LogMessage> LoggingToolbox::memory_buffer_;
std::mutex LoggingToolbox::logging_mutex_;
bool LoggingToolbox::is_initialized_ = false;

// =====================================================================================
// INITIALIZATION AND CONFIGURATION
// =====================================================================================

bool LoggingToolbox::Initialize(const LoggingConfiguration& config) {
    std::lock_guard<std::mutex> lock(logging_mutex_);

    if (is_initialized_) {
        return true; // Already initialized
    }

    if (!ValidateConfiguration(config)) {
        return false;
    }

    global_config_ = config;

    // Initialize file logging if enabled
    if ((static_cast<int>(config.outputs) & static_cast<int>(LogOutput::FILE)) && !config.log_file_path.empty()) {
        log_file_stream_.open(config.log_file_path, std::ios::app);
        if (!log_file_stream_.is_open()) {
            return false;
        }
    }

    is_initialized_ = true;
    return true;
}

LoggingConfiguration LoggingToolbox::GetConfiguration() {
    std::lock_guard<std::mutex> lock(logging_mutex_);
    return global_config_;
}

bool LoggingToolbox::UpdateConfiguration(const LoggingConfiguration& config) {
    std::lock_guard<std::mutex> lock(logging_mutex_);

    if (!ValidateConfiguration(config)) {
        return false;
    }

    // Close existing file if path changed
    if (!config.log_file_path.empty() && config.log_file_path != global_config_.log_file_path) {
        if (log_file_stream_.is_open()) {
            log_file_stream_.close();
        }
        log_file_stream_.open(config.log_file_path, std::ios::app);
    }

    global_config_ = config;
    return true;
}

bool LoggingToolbox::IsInitialized() {
    std::lock_guard<std::mutex> lock(logging_mutex_);
    return is_initialized_;
}

// =====================================================================================
// LOGGING OPERATIONS
// =====================================================================================

bool LoggingToolbox::Log(LogLevel level, const std::string& message,
                        const std::string& component, const std::string& function,
                        const std::string& file, uint32_t line) {
    std::lock_guard<std::mutex> lock(logging_mutex_);

    if (!is_initialized_) {
        InitializeDefaultConfiguration();
    }

    if (!ShouldLog(level)) {
        return true; // Silently ignore if level is below minimum
    }

    // Create log message structure
    LogMessage log_msg;
    log_msg.level = level;
    log_msg.message = message;
    log_msg.component = component;
    log_msg.function = function;
    log_msg.file = file;
    log_msg.line = line;
    log_msg.timestamp = GetCurrentTimestamp();
    log_msg.thread_id = GetCurrentThreadId();
    log_msg.process_id = GetCurrentProcessId();

    // Format message for each enabled output
    std::string formatted_message = FormatMessage(log_msg);

    if (static_cast<int>(global_config_.outputs) & static_cast<int>(LogOutput::CONSOLE)) {
        WriteToConsole(formatted_message);
    }

    if (static_cast<int>(global_config_.outputs) & static_cast<int>(LogOutput::FILE) && log_file_stream_.is_open()) {
        WriteToFile(formatted_message);
    }

    if (static_cast<int>(global_config_.outputs) & static_cast<int>(LogOutput::MEMORY)) {
        WriteToMemory(log_msg);
    }

    return true;
}

bool LoggingToolbox::Trace(const std::string& message, const std::string& component, const std::string& function) {
    return Log(LogLevel::TRACE, message, component, function);
}

bool LoggingToolbox::Debug(const std::string& message, const std::string& component, const std::string& function) {
    return Log(LogLevel::DEBUG, message, component, function);
}

bool LoggingToolbox::Info(const std::string& message, const std::string& component, const std::string& function) {
    return Log(LogLevel::INFO, message, component, function);
}

bool LoggingToolbox::Warn(const std::string& message, const std::string& component, const std::string& function) {
    return Log(LogLevel::WARN, message, component, function);
}

bool LoggingToolbox::Error(const std::string& message, const std::string& component, const std::string& function) {
    return Log(LogLevel::ERR, message, component, function);
}

bool LoggingToolbox::Critical(const std::string& message, const std::string& component, const std::string& function) {
    return Log(LogLevel::CRITICAL, message, component, function);
}

// =====================================================================================
// BATCH LOGGING
// =====================================================================================

bool LoggingToolbox::LogBatch(const std::vector<LogMessage>& messages) {
    std::lock_guard<std::mutex> lock(logging_mutex_);

    if (!is_initialized_) {
        InitializeDefaultConfiguration();
    }

    for (const auto& msg : messages) {
        if (ShouldLog(msg.level)) {
            std::string formatted_message = FormatMessage(msg);

            if (static_cast<int>(global_config_.outputs) & static_cast<int>(LogOutput::CONSOLE)) {
                WriteToConsole(formatted_message);
            }

            if (static_cast<int>(global_config_.outputs) & static_cast<int>(LogOutput::FILE) && log_file_stream_.is_open()) {
                WriteToFile(formatted_message);
            }

            if (static_cast<int>(global_config_.outputs) & static_cast<int>(LogOutput::MEMORY)) {
                WriteToMemory(msg);
            }
        }
    }

    return true;
}

bool LoggingToolbox::LogBatch(LogLevel level, const std::vector<std::string>& messages, const std::string& component) {
    std::lock_guard<std::mutex> lock(logging_mutex_);

    if (!is_initialized_) {
        InitializeDefaultConfiguration();
    }

    if (!ShouldLog(level)) {
        return true;
    }

    for (const auto& message : messages) {
        LogMessage log_msg;
        log_msg.level = level;
        log_msg.message = message;
        log_msg.component = component;
        log_msg.timestamp = GetCurrentTimestamp();
        log_msg.thread_id = GetCurrentThreadId();
        log_msg.process_id = GetCurrentProcessId();

        std::string formatted_message = FormatMessage(log_msg);

        if (static_cast<int>(global_config_.outputs) & static_cast<int>(LogOutput::CONSOLE)) {
            WriteToConsole(formatted_message);
        }
    
        if (static_cast<int>(global_config_.outputs) & static_cast<int>(LogOutput::FILE) && log_file_stream_.is_open()) {
            WriteToFile(formatted_message);
        }
    
        if (static_cast<int>(global_config_.outputs) & static_cast<int>(LogOutput::MEMORY)) {
            WriteToMemory(log_msg);
        }
    }

    return true;
}

// =====================================================================================
// FILE MANAGEMENT
// =====================================================================================

bool LoggingToolbox::OpenLogFile(const std::string& file_path) {
    std::lock_guard<std::mutex> lock(logging_mutex_);

    if (log_file_stream_.is_open()) {
        log_file_stream_.close();
    }

    log_file_stream_.open(file_path, std::ios::app);
    if (log_file_stream_.is_open()) {
        global_config_.log_file_path = file_path;
        return true;
    }

    return false;
}

bool LoggingToolbox::CloseLogFile() {
    std::lock_guard<std::mutex> lock(logging_mutex_);

    if (log_file_stream_.is_open()) {
        log_file_stream_.close();
        return true;
    }

    return false;
}

bool LoggingToolbox::RotateLogFile() {
    std::lock_guard<std::mutex> lock(logging_mutex_);

    if (!log_file_stream_.is_open()) {
        return false;
    }

    std::string current_path = global_config_.log_file_path;
    std::string backup_path = current_path + "." + std::to_string(GetCurrentTimestamp());

    log_file_stream_.close();

    // Rename current file to backup
    if (MoveFileA(current_path.c_str(), backup_path.c_str())) {
        // Open new file
        log_file_stream_.open(current_path, std::ios::app);
        return log_file_stream_.is_open();
    }

    return false;
}

size_t LoggingToolbox::CleanupOldLogFiles(size_t max_files) {
    std::lock_guard<std::mutex> lock(logging_mutex_);

    std::string base_path = global_config_.log_file_path;
    size_t dot_pos = base_path.find_last_of('.');

    if (dot_pos == std::string::npos) {
        return 0; // No extension found
    }

    std::string base_name = base_path.substr(0, dot_pos);
    std::string extension = base_path.substr(dot_pos);

    // This is a simplified implementation
    // In a real implementation, would scan directory for backup files
    return 0;
}

// =====================================================================================
// MEMORY BUFFER MANAGEMENT
// =====================================================================================

std::vector<LogMessage> LoggingToolbox::GetMemoryBuffer(size_t count) {
    std::lock_guard<std::mutex> lock(logging_mutex_);

    size_t start_pos = (memory_buffer_.size() > count) ? memory_buffer_.size() - count : 0;
    return std::vector<LogMessage>(memory_buffer_.begin() + start_pos, memory_buffer_.end());
}

size_t LoggingToolbox::ClearMemoryBuffer() {
    std::lock_guard<std::mutex> lock(logging_mutex_);

    size_t cleared_count = memory_buffer_.size();
    memory_buffer_.clear();
    return cleared_count;
}

size_t LoggingToolbox::GetMemoryBufferSize() {
    std::lock_guard<std::mutex> lock(logging_mutex_);
    return memory_buffer_.size();
}

// =====================================================================================
// FORMATTING AND OUTPUT
// =====================================================================================

std::string LoggingToolbox::FormatMessage(const LogMessage& msg) {
    std::ostringstream oss;

    // Add timestamp if enabled
    if (global_config_.enable_timestamp) {
        oss << CreateTimestampString() << " ";
    }

    // Add level
    oss << "[" << LogLevelToString(msg.level) << "] ";

    // Add component if enabled and available
    if (global_config_.enable_component && !msg.component.empty()) {
        oss << "[" << msg.component << "] ";
    }

    // Add function if enabled and available
    if (global_config_.enable_function && !msg.function.empty()) {
        oss << "(" << msg.function << ") ";
    }

    // Add message
    oss << msg.message;

    return oss.str();
}

std::string LoggingToolbox::FormatForConsole(LogLevel level, const std::string& message, const std::string& component) {
    std::ostringstream oss;

    // Add level with color coding (simplified)
    oss << "[" << LogLevelToString(level) << "] ";

    if (!component.empty()) {
        oss << "[" << component << "] ";
    }

    oss << message;

    return oss.str();
}

std::string LoggingToolbox::FormatForFile(LogLevel level, const std::string& message, const std::string& component) {
    std::ostringstream oss;

    oss << CreateTimestampString() << " ";
    oss << "[" << LogLevelToString(level) << "] ";

    if (!component.empty()) {
        oss << "[" << component << "] ";
    }

    oss << message;

    return oss.str();
}

// =====================================================================================
// UTILITY FUNCTIONS
// =====================================================================================

std::string LoggingToolbox::LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARN: return "WARN";
        case LogLevel::ERR: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

LogLevel LoggingToolbox::StringToLogLevel(const std::string& str) {
    std::string upper_str = str;
    std::transform(upper_str.begin(), upper_str.end(), upper_str.begin(), ::toupper);

    if (upper_str == "TRACE") return LogLevel::TRACE;
    if (upper_str == "DEBUG") return LogLevel::DEBUG;
    if (upper_str == "INFO") return LogLevel::INFO;
    if (upper_str == "WARN") return LogLevel::WARN;
    if (upper_str == "ERROR" || upper_str == "ERR") return LogLevel::ERR;
    if (upper_str == "CRITICAL") return LogLevel::CRITICAL;

    return LogLevel::INFO; // Default fallback
}

bool LoggingToolbox::ShouldLog(LogLevel level) {
    return static_cast<int>(level) >= static_cast<int>(global_config_.min_level);
}

uint32_t LoggingToolbox::GetCurrentThreadId() {
    return GetCurrentThreadId();
}

DWORD LoggingToolbox::GetCurrentProcessId() {
    return GetCurrentProcessId();
}

DWORD LoggingToolbox::GetCurrentTimestamp() {
    LogTimestamp now = TimestampToolbox::Now();
    return static_cast<DWORD>(TimestampToolbox::ToUnixTime(now));
}

// =====================================================================================
// PRIVATE HELPER METHODS
// =====================================================================================

void LoggingToolbox::WriteToConsole(const std::string& formatted_message) {
    std::cout << formatted_message << std::endl;
}

void LoggingToolbox::WriteToFile(const std::string& formatted_message) {
    if (log_file_stream_.is_open()) {
        log_file_stream_ << formatted_message << std::endl;
        log_file_stream_.flush();
    }
}

void LoggingToolbox::WriteToMemory(const LogMessage& message) {
    memory_buffer_.push_back(message);

    // Keep buffer size manageable
    if (memory_buffer_.size() > 1000) {
        memory_buffer_.erase(memory_buffer_.begin(), memory_buffer_.begin() + 200);
    }
}

std::string LoggingToolbox::CreateTimestampString() {
    time_t now = time(nullptr);
    struct tm tm_now;
    localtime_s(&tm_now, &now);

    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(4) << (tm_now.tm_year + 1900) << "-"
        << std::setw(2) << (tm_now.tm_mon + 1) << "-"
        << std::setw(2) << tm_now.tm_mday << " "
        << std::setw(2) << tm_now.tm_hour << ":"
        << std::setw(2) << tm_now.tm_min << ":"
        << std::setw(2) << tm_now.tm_sec;

    return oss.str();
}

LoggingConfiguration LoggingToolbox::CreateDefaultConfiguration() {
    LoggingConfiguration config;
    config.min_level = LogLevel::INFO;
    config.outputs = LogOutput::CONSOLE;
    config.log_file_path = "logs/asfm_logger.log";
    config.max_file_size = 10 * 1024 * 1024; // 10MB
    config.max_files = 5;
    config.enable_thread_safe = true;
    config.enable_timestamp = true;
    config.enable_component = true;
    config.enable_function = true;

    return config;
}

bool LoggingToolbox::ValidateConfiguration(const LoggingConfiguration& config) {
    return config.max_file_size > 0 &&
           config.max_files > 0 &&
           !config.log_file_path.empty();
}

void LoggingToolbox::InitializeDefaultConfiguration() {
    if (!is_initialized_) {
        global_config_ = CreateDefaultConfiguration();
        is_initialized_ = true;
    }
}