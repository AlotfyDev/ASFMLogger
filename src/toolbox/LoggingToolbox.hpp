#ifndef __ASFM_LOGGER_LOGGING_TOOLBOX_HPP__
#define __ASFM_LOGGER_LOGGING_TOOLBOX_HPP__

/**
 * ASFMLogger Logging Toolbox
 *
 * Static methods for robust logging operations following toolbox architecture.
 * Pure functions for thread-safe logging, file management, and output formatting.
 */

#include "structs/LogDataStructures.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <mutex>

enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERR = 4,
    CRITICAL = 5,
    NONE = 6
};

enum class LogOutput {
    CONSOLE = 1,
    FILE = 2,
    MEMORY = 4,
    ALL = CONSOLE | FILE | MEMORY
};

inline LogOutput operator|(LogOutput a, LogOutput b) {
    return static_cast<LogOutput>(static_cast<int>(a) | static_cast<int>(b));
}

inline LogOutput operator&(LogOutput a, LogOutput b) {
    return static_cast<LogOutput>(static_cast<int>(a) & static_cast<int>(b));
}

struct LogMessage {
    LogLevel level;
    std::string message;
    std::string component;
    std::string function;
    std::string file;
    uint32_t line;
    DWORD timestamp;
    uint32_t thread_id;
    uint32_t process_id;
};

struct LoggingConfiguration {
    LogLevel min_level;
    LogOutput outputs;
    std::string log_file_path;
    size_t max_file_size;
    size_t max_files;
    bool enable_thread_safe;
    bool enable_timestamp;
    bool enable_component;
    bool enable_function;
    char reserved[256];
};

class LoggingToolbox {
private:
    // Static logging state
    static LoggingConfiguration global_config_;
    static std::ofstream log_file_stream_;
    static std::vector<LogMessage> memory_buffer_;
    static std::mutex logging_mutex_;
    static bool is_initialized_;

public:
    // =================================================================================
    // INITIALIZATION AND CONFIGURATION
    // =================================================================================

    /**
     * @brief Initialize logging system with configuration
     * @param config Logging configuration
     * @return true if initialization successful
     */
    static bool Initialize(const LoggingConfiguration& config);

    /**
     * @brief Get current logging configuration
     * @return Current configuration
     */
    static LoggingConfiguration GetConfiguration();

    /**
     * @brief Update logging configuration
     * @param config New configuration
     * @return true if update successful
     */
    static bool UpdateConfiguration(const LoggingConfiguration& config);

    /**
     * @brief Check if logging system is initialized
     * @return true if initialized
     */
    static bool IsInitialized();

    // =================================================================================
    // LOGGING OPERATIONS
    // =================================================================================

    /**
     * @brief Log message with specified level
     * @param level Log level
     * @param message Message content
     * @param component Component name (optional)
     * @param function Function name (optional)
     * @param file Source file (optional)
     * @param line Source line number (optional)
     * @return true if logged successfully
     */
    static bool Log(LogLevel level, const std::string& message,
                   const std::string& component = "",
                   const std::string& function = "",
                   const std::string& file = "",
                   uint32_t line = 0);

    /**
     * @brief Log trace message
     * @param message Message content
     * @param component Component name (optional)
     * @param function Function name (optional)
     * @return true if logged successfully
     */
    static bool Trace(const std::string& message,
                     const std::string& component = "",
                     const std::string& function = "");

    /**
     * @brief Log debug message
     * @param message Message content
     * @param component Component name (optional)
     * @param function Function name (optional)
     * @return true if logged successfully
     */
    static bool Debug(const std::string& message,
                     const std::string& component = "",
                     const std::string& function = "");

    /**
     * @brief Log info message
     * @param message Message content
     * @param component Component name (optional)
     * @param function Function name (optional)
     * @return true if logged successfully
     */
    static bool Info(const std::string& message,
                    const std::string& component = "",
                    const std::string& function = "");

    /**
     * @brief Log warning message
     * @param message Message content
     * @param component Component name (optional)
     * @param function Function name (optional)
     * @return true if logged successfully
     */
    static bool Warn(const std::string& message,
                    const std::string& component = "",
                    const std::string& function = "");

    /**
     * @brief Log error message
     * @param message Message content
     * @param component Component name (optional)
     * @param function Function name (optional)
     * @return true if logged successfully
     */
    static bool Error(const std::string& message,
                     const std::string& component = "",
                     const std::string& function = "");

    /**
     * @brief Log critical message
     * @param message Message content
     * @param component Component name (optional)
     * @param function Function name (optional)
     * @return true if logged successfully
     */
    static bool Critical(const std::string& message,
                        const std::string& component = "",
                        const std::string& function = "");

    // =================================================================================
    // BATCH LOGGING
    // =================================================================================

    /**
     * @brief Log multiple messages in batch
     * @param messages Vector of log messages
     * @return true if batch logged successfully
     */
    static bool LogBatch(const std::vector<LogMessage>& messages);

    /**
     * @brief Log messages with same level in batch
     * @param level Log level for all messages
     * @param messages Vector of message strings
     * @param component Component name (optional)
     * @return true if batch logged successfully
     */
    static bool LogBatch(LogLevel level, const std::vector<std::string>& messages,
                        const std::string& component = "");

    // =================================================================================
    // FILE MANAGEMENT
    // =================================================================================

    /**
     * @brief Open log file for writing
     * @param file_path Path to log file
     * @return true if file opened successfully
     */
    static bool OpenLogFile(const std::string& file_path);

    /**
     * @brief Close current log file
     * @return true if file closed successfully
     */
    static bool CloseLogFile();

    /**
     * @brief Rotate log file (create new file, archive old)
     * @return true if rotation successful
     */
    static bool RotateLogFile();

    /**
     * @brief Clean up old log files
     * @param max_files Maximum number of files to keep
     * @return Number of files cleaned up
     */
    static size_t CleanupOldLogFiles(size_t max_files = 5);

    // =================================================================================
    // MEMORY BUFFER MANAGEMENT
    // =================================================================================

    /**
     * @brief Get messages from memory buffer
     * @param count Maximum number of messages to retrieve
     * @return Vector of recent log messages
     */
    static std::vector<LogMessage> GetMemoryBuffer(size_t count = 100);

    /**
     * @brief Clear memory buffer
     * @return Number of messages cleared
     */
    static size_t ClearMemoryBuffer();

    /**
     * @brief Get memory buffer size
     * @return Number of messages in buffer
     */
    static size_t GetMemoryBufferSize();

    // =================================================================================
    // FORMATTING AND OUTPUT
    // =================================================================================

    /**
     * @brief Format log message for output
     * @param msg Log message to format
     * @return Formatted message string
     */
    static std::string FormatMessage(const LogMessage& msg);

    /**
     * @brief Format message for console output
     * @param level Log level
     * @param message Message content
     * @param component Component name
     * @return Console-formatted string
     */
    static std::string FormatForConsole(LogLevel level, const std::string& message,
                                       const std::string& component = "");

    /**
     * @brief Format message for file output
     * @param level Log level
     * @param message Message content
     * @param component Component name
     * @return File-formatted string
     */
    static std::string FormatForFile(LogLevel level, const std::string& message,
                                    const std::string& component = "");

    // =================================================================================
    // UTILITY FUNCTIONS
    // =================================================================================

    /**
     * @brief Convert LogLevel to string
     * @param level Log level
     * @return String representation
     */
    static std::string LogLevelToString(LogLevel level);

    /**
     * @brief Convert string to LogLevel
     * @param str String representation
     * @return Corresponding LogLevel
     */
    static LogLevel StringToLogLevel(const std::string& str);

    /**
     * @brief Check if level should be logged
     * @param level Level to check
     * @return true if level should be logged
     */
    static bool ShouldLog(LogLevel level);

    /**
     * @brief Get current thread ID
     * @return Thread ID
     */
    static uint32_t GetCurrentThreadId();

    /**
     * @brief Get current process ID
     * @return Process ID
     */
    static DWORD GetCurrentProcessId();

    /**
     * @brief Get current timestamp
     * @return Current timestamp in seconds
     */
    static DWORD GetCurrentTimestamp();

private:
    // Private helper methods
    static void WriteToConsole(const std::string& formatted_message);
    static void WriteToFile(const std::string& formatted_message);
    static void WriteToMemory(const LogMessage& message);
    static std::string CreateTimestampString();
    static LoggingConfiguration CreateDefaultConfiguration();
    static bool ValidateConfiguration(const LoggingConfiguration& config);
    static void InitializeDefaultConfiguration();
};

#endif // __ASFM_LOGGER_LOGGING_TOOLBOX_HPP__