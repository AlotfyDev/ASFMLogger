/**
 * Enhanced ASFMLogger Implementation
 *
 * This file contains the enhanced implementation of the Logger class
 * that integrates all the new features while maintaining backward compatibility.
 */

#include "../ASFMLogger.hpp"
#include "../src/structs/LogDataStructures.hpp"
#include "../src/structs/ImportanceConfiguration.hpp"
#include "../src/structs/PersistencePolicy.hpp"
#include "../src/toolbox/LogMessageToolbox.hpp"
#include "../src/toolbox/LoggerInstanceToolbox.hpp"
#include "../src/managers/LoggerInstanceManager.hpp"
#include "../src/stateful/LoggerInstance.hpp"
#include "../src/stateful/ImportanceMapper.hpp"
#include "../src/stateful/SmartMessageQueue.hpp"
#include "../src/stateful/DatabaseLogger.hpp"
#include "../src/managers/ContextualPersistenceManager.hpp"
#include <sstream>
#include <algorithm>

// =====================================================================================
// ENHANCED LOGGER IMPLEMENTATION
// =====================================================================================

void Logger::initializeEnhancedFeatures() {
    try {
        // Initialize logger instance
        instance_ = LoggerInstance::Create(application_name_, process_name_);

        // Register with global instance manager
        auto& instance_manager = ::GetGlobalInstanceManager();
        instance_manager.registerInstance(instance_);

        // Initialize importance mapper
        importance_mapper_ = new ImportanceMapper(application_name_);

        // Initialize persistence manager
        persistence_manager_ = new ContextualPersistenceManager(application_name_, importance_mapper_);

        // Initialize smart queue
        smart_queue_ = new SmartMessageQueue("main_queue", application_name_);

        // Initialize database logger (if enabled)
        database_logger_ = nullptr;

        enhanced_features_enabled_ = true;

        if (logger_) {
            logger_->info("Enhanced ASFMLogger features initialized for application: {}", application_name_);
        }

    } catch (const std::exception& ex) {
        if (logger_) {
            logger_->error("Failed to initialize enhanced features: {}", ex.what());
        }
        enhanced_features_enabled_ = false;
    }
}

void Logger::configureEnhanced(
    const std::string& application_name,
    bool enable_database_logging,
    const std::string& database_connection_string,
    bool enable_shared_memory,
    const std::string& shared_memory_name,
    bool console_output,
    const std::string& log_file_name,
    size_t max_file_size,
    size_t max_files,
    spdlog::level::level_enum log_level) {

    // Set application info
    application_name_ = application_name;
    process_name_ = ""; // Will be auto-detected

    // Initialize enhanced features
    initializeEnhancedFeatures();

    // Enable database logging if requested
    if (enable_database_logging && !database_connection_string.empty()) {
        enableDatabaseLogging(database_connection_string);
    }

    // Enable shared memory if requested
    if (enable_shared_memory && !shared_memory_name.empty()) {
        enableSharedMemoryLogging(shared_memory_name);
    }

    // Configure basic logging (existing functionality)
    configure(console_output, log_file_name, max_file_size, max_files, log_level);

    if (logger_) {
        logger_->info("Enhanced ASFMLogger configured for application: {}", application_name);
    }
}

// =====================================================================================
// ENHANCED LOGGING METHODS IMPLEMENTATION
// =====================================================================================

template<typename... Args>
void Logger::log(const std::string& level, const std::string& component, const std::string& function,
                 const std::string& fmt, Args&&... args) {
    try {
        // Format the message
        std::ostringstream oss;
        oss << fmt;
        int dummy[] = { 0, ((void)(oss << " " << args), 0)... };
        (void)dummy;
        std::string formatted_msg = oss.str();

        // Create enhanced log message data
        LogMessageData message_data;
        strncpy(message_data.message, formatted_msg.c_str(), sizeof(message_data.message) - 1);
        message_data.message[sizeof(message_data.message) - 1] = '\0';
        strncpy(message_data.component, component.c_str(), sizeof(message_data.component) - 1);
        message_data.component[sizeof(message_data.component) - 1] = '\0';
        strncpy(message_data.function, function.c_str(), sizeof(message_data.function) - 1);
        message_data.function[sizeof(message_data.function) - 1] = '\0';
        message_data.type = LogMessageType::INFO;

        // Update instance activity
        if (enhanced_features_enabled_) {
            instance_.incrementMessageCount();
            instance_.updateActivity();
        }

        // Route to appropriate destinations based on importance
        if (enhanced_features_enabled_ && importance_mapper_ && persistence_manager_) {
            // Use enhanced routing
            routeEnhancedMessage(message_data);
        } else {
            // Use basic routing
            if (logger_) {
                if (level == "trace") logger_->trace(formatted_msg);
                else if (level == "debug") logger_->debug(formatted_msg);
                else if (level == "info") logger_->info(formatted_msg);
                else if (level == "warn") logger_->warn(formatted_msg);
                else if (level == "error") logger_->error(formatted_msg);
                else if (level == "critical") logger_->critical(formatted_msg);
            }
        }

    } catch (const std::exception& ex) {
        if (logger_) {
            logger_->error("Exception in enhanced logging: {}", ex.what());
        }
    }
}

template<typename... Args>
void Logger::trace(const std::string& component, const std::string& function,
                   const std::string& fmt, Args&&... args) {
    log("trace", component, function, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void Logger::debug(const std::string& component, const std::string& function,
                   const std::string& fmt, Args&&... args) {
    log("debug", component, function, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void Logger::info(const std::string& component, const std::string& function,
                  const std::string& fmt, Args&&... args) {
    log("info", component, function, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void Logger::warn(const std::string& component, const std::string& function,
                  const std::string& fmt, Args&&... args) {
    log("warn", component, function, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void Logger::error(const std::string& component, const std::string& function,
                   const std::string& fmt, Args&&... args) {
    log("error", component, function, fmt, std::forward<Args>(args)...);

    // Also call legacy error storage
    std::ostringstream oss;
    oss << fmt;
    int dummy[] = { 0, ((void)(oss << " " << args), 0)... };
    (void)dummy;
    storeLastError(oss.str());
}

template<typename... Args>
void Logger::critical(const std::string& component, const std::string& function,
                      const std::string& fmt, Args&&... args) {
    log("critical", component, function, fmt, std::forward<Args>(args)...);

    // Also call legacy error storage
    std::ostringstream oss;
    oss << fmt;
    int dummy[] = { 0, ((void)(oss << " " << args), 0)... };
    (void)dummy;
    storeLastError(oss.str());
}

// =====================================================================================
// ENHANCED MESSAGE ROUTING
// =====================================================================================

void Logger::routeEnhancedMessage(const LogMessageData& message_data) {
    try {
        // Update statistics
        total_messages_processed_++;

        // Determine message importance
        ImportanceResolutionContext context;
        strncpy(context.component, message_data.component, sizeof(context.component) - 1);
        context.component[sizeof(context.component) - 1] = '\0';
        strncpy(context.function, message_data.function, sizeof(context.function) - 1);
        context.function[sizeof(context.function) - 1] = '\0';
        strncpy(context.application_name, application_name_.c_str(), sizeof(context.application_name) - 1);
        context.application_name[sizeof(context.application_name) - 1] = '\0';
        context.message_type = message_data.type;

        auto importance_result = importance_mapper_->resolveMessageImportance(message_data, context);

        // Route to basic logger
        if (logger_) {
            std::string message_str = LogMessageToolbox::ExtractMessage(message_data);
            switch (message_data.type) {
                case LogMessageType::TRACE:
                    logger_->trace(message_str);
                    break;
                case LogMessageType::DEBUG:
                    logger_->debug(message_str);
                    break;
                case LogMessageType::INFO:
                    logger_->info(message_str);
                    break;
                case LogMessageType::WARN:
                    logger_->warn(message_str);
                    break;
                case LogMessageType::ERR:
                    logger_->error(message_str);
                    break;
                case LogMessageType::CRITICAL_LOG:
                    logger_->critical(message_str);
                    break;
            }
        }

        // Route to smart queue for intelligent buffering
        if (smart_queue_) {
            PersistenceDecisionContext persistence_context;
            strncpy(persistence_context.application_name, application_name_.c_str(),
                    sizeof(persistence_context.application_name) - 1);
            persistence_context.application_name[sizeof(persistence_context.application_name) - 1] = '\0';
            strncpy(persistence_context.component, message_data.component,
                    sizeof(persistence_context.component) - 1);
            persistence_context.component[sizeof(persistence_context.component) - 1] = '\0';
            persistence_context.message_type = message_data.type;
            persistence_context.resolved_importance = importance_result.final_importance;

            smart_queue_->enqueue(message_data, persistence_context);
        }

        // Route to database if enabled
        if (database_logging_enabled_ && database_logger_) {
            database_logger_->insertMessage(message_data);
            database_messages_persisted_++;
        }

        // Route to shared memory if enabled
        if (shared_memory_enabled_) {
            // TODO: Implement shared memory routing
            shared_memory_messages_sent_++;
        }

    } catch (const std::exception& ex) {
        if (logger_) {
            logger_->error("Exception in enhanced message routing: {}", ex.what());
        }
    }
}

// =====================================================================================
// ENHANCED FEATURES CONTROL
// =====================================================================================

bool Logger::enableDatabaseLogging(const std::string& connection_string) {
    try {
        if (!database_logger_) {
            database_logger_ = new DatabaseLogger(connection_string);
        }

        if (database_logger_->connect()) {
            database_logging_enabled_ = true;
            if (logger_) {
                logger_->info("Database logging enabled successfully");
            }
            return true;
        }

        return false;

    } catch (const std::exception& ex) {
        if (logger_) {
            logger_->error("Failed to enable database logging: {}", ex.what());
        }
        return false;
    }
}

bool Logger::disableDatabaseLogging() {
    try {
        if (database_logger_) {
            database_logger_->disconnect();
            delete database_logger_;
            database_logger_ = nullptr;
        }

        database_logging_enabled_ = false;

        if (logger_) {
            logger_->info("Database logging disabled");
        }

        return true;

    } catch (const std::exception& ex) {
        if (logger_) {
            logger_->error("Failed to disable database logging: {}", ex.what());
        }
        return false;
    }
}

bool Logger::enableSharedMemoryLogging(const std::string& shared_memory_name) {
    try {
        // TODO: Implement shared memory logging
        shared_memory_enabled_ = true;

        if (logger_) {
            logger_->info("Shared memory logging enabled for: {}", shared_memory_name);
        }

        return true;

    } catch (const std::exception& ex) {
        if (logger_) {
            logger_->error("Failed to enable shared memory logging: {}", ex.what());
        }
        return false;
    }
}

bool Logger::disableSharedMemoryLogging() {
    try {
        // TODO: Implement shared memory cleanup
        shared_memory_enabled_ = false;

        if (logger_) {
            logger_->info("Shared memory logging disabled");
        }

        return true;

    } catch (const std::exception& ex) {
        if (logger_) {
            logger_->error("Failed to disable shared memory logging: {}", ex.what());
        }
        return false;
    }
}

size_t Logger::flushAllQueues() {
    size_t total_flushed = 0;

    try {
        // Flush smart queue
        if (smart_queue_) {
            std::vector<LogMessageData> messages;
            size_t queue_size = smart_queue_->size();

            for (size_t i = 0; i < queue_size; ++i) {
                LogMessageData message;
                QueueEntryMetadata metadata;
                if (smart_queue_->dequeue(message, metadata)) {
                    messages.push_back(message);
                }
            }

            // Send to database if enabled
            if (database_logging_enabled_ && database_logger_ && !messages.empty()) {
                auto result = database_logger_->insertMessageBatch(messages);
                if (result.success) {
                    total_flushed += messages.size();
                }
            }
        }

        if (logger_) {
            logger_->info("Flushed {} messages from all queues", total_flushed);
        }

    } catch (const std::exception& ex) {
        if (logger_) {
            logger_->error("Exception during queue flush: {}", ex.what());
        }
    }

    return total_flushed;
}

// =====================================================================================
// STATISTICS AND MONITORING
// =====================================================================================

std::string Logger::getComprehensiveStatistics() const {
    std::ostringstream oss;

    oss << "=== ASFMLogger Comprehensive Statistics ===\n";
    oss << "Application: " << application_name_ << "\n";
    oss << "Process: " << process_name_ << "\n";
    oss << "Enhanced Features: " << (enhanced_features_enabled_ ? "Enabled" : "Disabled") << "\n";
    oss << "Database Logging: " << (database_logging_enabled_ ? "Enabled" : "Disabled") << "\n";
    oss << "Shared Memory: " << (shared_memory_enabled_ ? "Enabled" : "Disabled") << "\n";

    oss << "\n--- Message Statistics ---\n";
    oss << "Total Messages Processed: " << total_messages_processed_ << "\n";
    oss << "Database Messages Persisted: " << database_messages_persisted_ << "\n";
    oss << "Shared Memory Messages Sent: " << shared_memory_messages_sent_ << "\n";
    oss << "Queue Overflow Events: " << queue_overflow_events_ << "\n";

    oss << "\n--- Instance Information ---\n";
    if (enhanced_features_enabled_) {
        oss << instance_.toString() << "\n";
    }

    oss << "\n--- Component Statistics ---\n";
    if (importance_mapper_) {
        oss << importance_mapper_->getMappingStatistics() << "\n";
    }

    oss << "\n--- Queue Statistics ---\n";
    if (smart_queue_) {
        auto stats = smart_queue_->getStatistics();
        oss << "Queue Size: " << stats.current_queue_size << "\n";
        oss << "Total Queued: " << stats.total_messages_queued << "\n";
        oss << "Total Dequeued: " << stats.total_messages_dequeued << "\n";
        oss << "Total Evicted: " << stats.total_messages_evicted << "\n";
    }

    return oss.str();
}

std::string Logger::getStatus() const {
    std::ostringstream oss;

    oss << "ASFMLogger Status: ";
    oss << (is_configured_ ? "Configured" : "Not Configured");
    oss << ", Enhanced: " << (enhanced_features_enabled_ ? "Enabled" : "Disabled");
    oss << ", Database: " << (database_logging_enabled_ ? "Enabled" : "Disabled");
    oss << ", Shared Memory: " << (shared_memory_enabled_ ? "Enabled" : "Disabled");

    return oss.str();
}

// =====================================================================================
// EXPLICIT TEMPLATE INSTANTIATIONS
// =====================================================================================

// This is needed for templates to compile properly
template void Logger::trace(const std::string& component, const std::string& function,
                           const std::string& fmt);
template void Logger::debug(const std::string& component, const std::string& function,
                           const std::string& fmt);
template void Logger::info(const std::string& component, const std::string& function,
                          const std::string& fmt);
template void Logger::warn(const std::string& component, const std::string& function,
                          const std::string& fmt);
template void Logger::error(const std::string& component, const std::string& function,
                           const std::string& fmt);
template void Logger::critical(const std::string& component, const std::string& function,
                              const std::string& fmt);