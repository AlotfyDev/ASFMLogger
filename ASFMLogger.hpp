#ifndef __ASFMLOGGER_HPP__
#define __ASFMLOGGER_HPP__

/**
 * Enhanced ASFMLogger - Enterprise-Grade Logging Framework
 *
 * This enhanced version maintains full backward compatibility with the original ASFMLogger
 * while adding advanced features for enterprise applications:
 *
 * ENHANCED FEATURES:
 * - Multi-instance logging with application tracking
 * - Smart message classification with importance framework
 * - Contextual persistence with adaptive policies
 * - Intelligent queue management with priority preservation
 * - SQL Server integration for enterprise database logging
 * - Windows FileMaps support for inter-process communication
 *
 * BACKWARD COMPATIBILITY:
 * - All existing APIs work exactly as before
 * - No breaking changes to existing code
 * - Enhanced features are opt-in
 */

// Include spdlog headers - required for proper compilation
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/common.h>

// Include enhanced ASFMLogger components
#include "structs/LogDataStructures.hpp"
#include "structs/ImportanceConfiguration.hpp"
#include "structs/PersistencePolicy.hpp"
#include "structs/DatabaseConfiguration.hpp"
#include "structs/SmartQueueConfiguration.hpp"
#include "stateful/LoggerInstance.hpp"
#include "stateful/ImportanceMapper.hpp"
#include "managers/ContextualPersistenceManager.hpp"
#include "stateful/SmartMessageQueue.hpp"
#include "managers/MessageQueueManager.hpp"
#include "stateful/DatabaseLogger.hpp"



class Logger {
public:
    // Delete copy constructor and assignment operator to enforce singleton
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Get the singleton instance (enhanced with application support)
    static std::shared_ptr<Logger> getInstance(const std::string& application_name = "ASFMLogger",
                                              const std::string& process_name = "");

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

    // =================================================================================
    // ENHANCED LOGGING METHODS (New Features)
    // =================================================================================

    /**
     * @brief Enhanced logging with component and function tracking
     * @param level Log level (trace, debug, info, warn, error, critical)
     * @param component Component name
     * @param function Function name
     * @param fmt Format string
     * @param args Arguments for format string
     */
    template<typename... Args>
    void log(const std::string& level, const std::string& component, const std::string& function,
             const std::string& fmt, Args&&... args);

    /**
     * @brief Enhanced trace logging with component tracking
     * @param component Component name
     * @param function Function name
     * @param fmt Format string
     * @param args Arguments for format string
     */
    template<typename... Args>
    void trace(const std::string& component, const std::string& function,
               const std::string& fmt, Args&&... args);

    /**
     * @brief Enhanced debug logging with component tracking
     * @param component Component name
     * @param function Function name
     * @param fmt Format string
     * @param args Arguments for format string
     */
    template<typename... Args>
    void debug(const std::string& component, const std::string& function,
               const std::string& fmt, Args&&... args);

    /**
     * @brief Enhanced info logging with component tracking
     * @param component Component name
     * @param function Function name
     * @param fmt Format string
     * @param args Arguments for format string
     */
    template<typename... Args>
    void info(const std::string& component, const std::string& function,
              const std::string& fmt, Args&&... args);

    /**
     * @brief Enhanced warning logging with component tracking
     * @param component Component name
     * @param function Function name
     * @param fmt Format string
     * @param args Arguments for format string
     */
    template<typename... Args>
    void warn(const std::string& component, const std::string& function,
              const std::string& fmt, Args&&... args);

    /**
     * @brief Enhanced error logging with component tracking
     * @param component Component name
     * @param function Function name
     * @param fmt Format string
     * @param args Arguments for format string
     */
    template<typename... Args>
    void error(const std::string& component, const std::string& function,
               const std::string& fmt, Args&&... args);

    /**
     * @brief Enhanced critical logging with component tracking
     * @param component Component name
     * @param function Function name
     * @param fmt Format string
     * @param args Arguments for format string
     */
    template<typename... Args>
    void critical(const std::string& component, const std::string& function,
                  const std::string& fmt, Args&&... args);

    // =================================================================================
    // ENHANCED FEATURES ACCESS
    // =================================================================================

    /**
     * @brief Get logger instance information
     * @return LoggerInstance object with current instance details
     */
    LoggerInstance getInstanceInfo() const { return instance_; }

    /**
     * @brief Get importance mapper for this logger
     * @return ImportanceMapper object for message classification
     */
    ImportanceMapper* getImportanceMapper() const { return importance_mapper_; }

    /**
     * @brief Get persistence manager for this logger
     * @return ContextualPersistenceManager for persistence decisions
     */
    ContextualPersistenceManager* getPersistenceManager() const { return persistence_manager_; }

    /**
     * @brief Get smart queue for this logger
     * @return SmartMessageQueue for intelligent message buffering
     */
    SmartMessageQueue* getSmartQueue() const { return smart_queue_; }

    /**
     * @brief Get database logger for this logger
     * @return DatabaseLogger for SQL Server operations
     */
    DatabaseLogger* getDatabaseLogger() const { return database_logger_; }

    /**
     * @brief Enable database logging
     * @param connection_string Database connection string
     * @return true if database logging enabled successfully
     */
    bool enableDatabaseLogging(const std::string& connection_string);

    /**
     * @brief Disable database logging
     * @return true if database logging disabled successfully
     */
    bool disableDatabaseLogging();

    /**
     * @brief Enable shared memory logging
     * @param shared_memory_name Name for shared memory mapping
     * @return true if shared memory logging enabled successfully
     */
    bool enableSharedMemoryLogging(const std::string& shared_memory_name);

    /**
     * @brief Disable shared memory logging
     * @return true if shared memory logging disabled successfully
     */
    bool disableSharedMemoryLogging();

    /**
     * @brief Flush all queued messages to their destinations
     * @return Number of messages flushed
     */
    size_t flushAllQueues();

    /**
     * @brief Get comprehensive logger statistics
     * @return Formatted string with all logger statistics
     */
    std::string getComprehensiveStatistics() const;

    /**
     * @brief Get logger status information
     * @return Formatted string with current logger status
     */
    std::string getStatus() const;

    // Configure the logger (enhanced with new features)
    void configure(
        bool console_output = true,
        const std::string& log_file_name = "framework.log",
        size_t max_file_size = 10485760, // 10 MB
        size_t max_files = 5,
        spdlog::level::level_enum log_level = spdlog::level::info
    );

    // Enhanced configuration with full feature support
    void configureEnhanced(
        const std::string& application_name,
        bool enable_database_logging = false,
        const std::string& database_connection_string = "",
        bool enable_shared_memory = false,
        const std::string& shared_memory_name = "",
        bool console_output = true,
        const std::string& log_file_name = "framework.log",
        size_t max_file_size = 10485760, // 10 MB
        size_t max_files = 5,
        spdlog::level::level_enum log_level = spdlog::level::info
    );

    // Check if logger is configured
    bool isConfigured() const { return is_configured_; }

public:
    // Public constructor and destructor for std::make_shared
    Logger();
    Logger(const std::string& application_name, const std::string& process_name);
    ~Logger();

    // Store the last error message
    template<typename... Args>
    void storeLastError(const std::string& fmt, Args&&... args);

    // =================================================================================
    // ENHANCED FEATURES (New Private Members)
    // =================================================================================

    // Core enhanced components
    LoggerInstance instance_;                          // Logger instance information
    ImportanceMapper* importance_mapper_;              // Message importance classification
    ContextualPersistenceManager* persistence_manager_; // Persistence decision making
    SmartMessageQueue* smart_queue_;                   // Intelligent message buffering
    DatabaseLogger* database_logger_;                  // SQL Server integration

    // Enhanced state tracking
    bool enhanced_features_enabled_;                   // Whether enhanced features are active
    bool database_logging_enabled_;                    // Whether database logging is active
    bool shared_memory_enabled_;                       // Whether shared memory is active
    std::string application_name_;                     // Application identifier
    std::string process_name_;                         // Process identifier

    // Performance monitoring
    std::atomic<uint64_t> total_messages_processed_;   // Total messages processed
    std::atomic<uint64_t> database_messages_persisted_; // Messages sent to database
    std::atomic<uint64_t> shared_memory_messages_sent_;  // Messages sent to shared memory
    std::atomic<uint64_t> queue_overflow_events_;      // Queue overflow incidents

    // Enhanced features initialization
    void initializeEnhancedFeatures();
    void routeEnhancedMessage(const LogMessageData& message_data);

    // Legacy members (maintained for backward compatibility)
    std::shared_ptr<spdlog::logger> logger_;
    bool is_configured_ = false;
    static std::string last_error_; // Store last error message
    static std::mutex error_mutex_; // Mutex for thread-safe access to last_error_
};











#endif // __ASFMLOGGER_H__