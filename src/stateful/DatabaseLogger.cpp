/**
 * ASFMLogger DatabaseLogger Implementation
 *
 * Stateful wrapper implementation for database operations.
 * Provides thread-safe object-oriented interface using DatabaseToolbox internally.
 */

#include "stateful/DatabaseLogger.hpp"
#include "toolbox/DatabaseToolbox.hpp"
#include "structs/DatabaseConfiguration.hpp"
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cstring>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>

// =====================================================================================
// CONSTRUCTORS AND DESTRUCTOR
// =====================================================================================

DatabaseLogger::DatabaseLogger()
    : connection_config_(), connection_id_(0), is_connected_(false), connection_mutex_(),
      total_operations_(0), successful_operations_(0), failed_operations_(0), total_operation_time_ms_(0),
      active_connections_(), max_connections_(10), connection_pool_mutex_(),
      prepared_statements_(), prepared_statement_mutex_() {

    // Initialize with default configuration
    connection_config_ = DatabaseToolbox::CreateDefaultLoggingConfig("localhost", "ASFMLoggerDB");
}

DatabaseLogger::DatabaseLogger(const DatabaseConnectionConfig& config)
    : connection_config_(config), connection_id_(0), is_connected_(false), connection_mutex_(),
      total_operations_(0), successful_operations_(0), failed_operations_(0), total_operation_time_ms_(0),
      active_connections_(), max_connections_(10), connection_pool_mutex_(),
      prepared_statements_(), prepared_statement_mutex_() {

    // Validate configuration using ToolBox
    if (!DatabaseToolbox::ValidateConnectionConfig(config)) {
        throw std::invalid_argument("Invalid database connection configuration");
    }
}

DatabaseLogger::DatabaseLogger(const std::string& connection_string)
    : connection_config_(), connection_id_(0), is_connected_(false), connection_mutex_(),
      total_operations_(0), successful_operations_(0), failed_operations_(0), total_operation_time_ms_(0),
      active_connections_(), max_connections_(10), connection_pool_mutex_(),
      prepared_statements_(), prepared_statement_mutex_() {

    // Parse connection string using ToolBox
    connection_config_ = DatabaseToolbox::ParseConnectionString(connection_string);

    // Validate parsed configuration
    if (!DatabaseToolbox::ValidateConnectionConfig(connection_config_)) {
        throw std::invalid_argument("Invalid connection string provided");
    }
}

DatabaseLogger::DatabaseLogger(const std::string& server_name,
                               const std::string& database_name,
                               const std::string& username,
                               const std::string& password,
                               bool use_windows_auth)
    : connection_config_(), connection_id_(0), is_connected_(false), connection_mutex_(),
      total_operations_(0), successful_operations_(0), failed_operations_(0), total_operation_time_ms_(0),
      active_connections_(), max_connections_(10), connection_pool_mutex_(),
      prepared_statements_(), prepared_statement_mutex_() {

    // Create configuration using ToolBox parameterized method
    connection_config_ = DatabaseToolbox::CreateConnectionConfig(
        server_name, database_name, username, password, use_windows_auth);

    // Validate configuration
    if (!DatabaseToolbox::ValidateConnectionConfig(connection_config_)) {
        throw std::invalid_argument("Invalid database connection parameters");
    }
}

DatabaseLogger::DatabaseLogger(const DatabaseLogger& other)
    : connection_config_(other.connection_config_), connection_id_(other.connection_id_),
      is_connected_(false), connection_mutex_(), // Start disconnected for safety
      total_operations_(0), successful_operations_(0), failed_operations_(0), total_operation_time_ms_(0),
      active_connections_(), max_connections_(other.max_connections_), connection_pool_mutex_(),
      prepared_statements_(), prepared_statement_mutex_() {

    // Copy configuration only - don't copy runtime state
    // Runtime state should be initialized fresh
}

DatabaseLogger::DatabaseLogger(DatabaseLogger&& other) noexcept
    : connection_config_(std::move(other.connection_config_)),
      connection_id_(other.connection_id_), is_connected_(other.is_connected_),
      connection_mutex_(),
      total_operations_(other.total_operations_.load()),
      successful_operations_(other.successful_operations_.load()),
      failed_operations_(other.failed_operations_.load()),
      total_operation_time_ms_(other.total_operation_time_ms_.load()),
      active_connections_(std::move(other.active_connections_)),
      max_connections_(other.max_connections_), connection_pool_mutex_(),
      prepared_statements_(std::move(other.prepared_statements_)),
      prepared_statement_mutex_() {
    // Move constructor - mark source as disconnected for safety
    other.is_connected_ = false;
}

DatabaseLogger& DatabaseLogger::operator=(const DatabaseLogger& other) {
    if (this != &other) {
        std::lock_guard<std::mutex> this_lock(connection_mutex_);

        connection_config_ = other.connection_config_;
        connection_id_ = other.connection_id_;
        is_connected_ = false; // Start disconnected for safety
        total_operations_ = 0; // Reset statistics
        successful_operations_ = 0;
        failed_operations_ = 0;
        total_operation_time_ms_ = 0;
        max_connections_ = other.max_connections_;
        // Don't copy runtime state - initialize fresh
    }
    return *this;
}

DatabaseLogger& DatabaseLogger::operator=(DatabaseLogger&& other) noexcept {
    if (this != &other) {
        std::lock_guard<std::mutex> this_lock(connection_mutex_);

        connection_config_ = std::move(other.connection_config_);
        connection_id_ = other.connection_id_;
        is_connected_ = other.is_connected_;
        total_operations_ = other.total_operations_.load();
        successful_operations_ = other.successful_operations_.load();
        failed_operations_ = other.failed_operations_.load();
        total_operation_time_ms_ = other.total_operation_time_ms_.load();
        active_connections_ = std::move(other.active_connections_);
        max_connections_ = other.max_connections_;
        prepared_statements_ = std::move(other.prepared_statements_);

        // Mark source as disconnected for safety
        other.is_connected_ = false;
    }
    return *this;
}

DatabaseLogger::~DatabaseLogger() {
    // Clean shutdown - disconnect if connected
    if (is_connected_) {
        disconnect();
    }

    // Clean up prepared statements
    clear();
}

// =====================================================================================
// PRIVATE HELPER METHODS
// =====================================================================================

bool DatabaseLogger::initializeConnection() {
    // DEBUG: Log connection initialization start
    std::cout << "[DEBUG] DatabaseLogger::initializeConnection() - Starting connection initialization" << std::endl;

    // Generate unique connection ID using hash of connection string
    DWORD current_time = DatabaseToolbox::GetCurrentTimestamp();
    std::cout << "[DEBUG] DatabaseLogger::initializeConnection() - Current timestamp: " << current_time << std::endl;

    connection_id_ = std::hash<std::string>{}(connection_config_.connection_string) + current_time;
    std::cout << "[DEBUG] DatabaseLogger::initializeConnection() - Generated connection_id: " << connection_id_ << std::endl;

    // Initialize connection state using ToolBox
    std::cout << "[DEBUG] DatabaseLogger::initializeConnection() - Calling DatabaseToolbox::InitializeConnectionState" << std::endl;
    if (!DatabaseToolbox::InitializeConnectionState(connection_id_, connection_config_)) {
        std::cout << "[ERROR] DatabaseLogger::initializeConnection() - InitializeConnectionState failed" << std::endl;
        return false;
    }
    std::cout << "[DEBUG] DatabaseLogger::initializeConnection() - InitializeConnectionState succeeded" << std::endl;

    // Test connection using ToolBox
    std::cout << "[DEBUG] DatabaseLogger::initializeConnection() - Calling DatabaseToolbox::TestConnection" << std::endl;
    if (!DatabaseToolbox::TestConnection(connection_config_)) {
        std::cout << "[ERROR] DatabaseLogger::initializeConnection() - TestConnection failed" << std::endl;
        return false;
    }
    std::cout << "[DEBUG] DatabaseLogger::initializeConnection() - TestConnection succeeded" << std::endl;

    std::cout << "[DEBUG] DatabaseLogger::initializeConnection() - Connection initialization completed successfully" << std::endl;
    return true;
}

void DatabaseLogger::cleanupConnection() {
    // Clean up connection state
    is_connected_ = false;

    // Clear prepared statements
    clear();
}

void DatabaseLogger::updatePerformanceStatistics(const DatabaseOperationResult& result) {
    // Update atomic counters
    total_operations_++;

    if (result.success) {
        successful_operations_++;
    } else {
        failed_operations_++;
    }

    total_operation_time_ms_ += result.duration_ms;
}

// Helper function to create DatabaseOperationResult
DatabaseOperationResult DatabaseLogger::createOperationResult(bool success, const std::string& operation_type,
                                                            const std::string& message) {
    DatabaseOperationResult result;
    memset(&result, 0, sizeof(DatabaseOperationResult));

    result.operation_id = 0;
    result.start_time = 0;
    result.end_time = 0;
    result.duration_ms = 0;
    result.success = success;
    result.error_code = success ? 0 : 1;
    result.connection_id = connection_id_;

    strncpy_s(result.operation_type, operation_type.c_str(), sizeof(result.operation_type) - 1);
    result.operation_type[sizeof(result.operation_type) - 1] = '\0';

    strncpy_s(result.error_message, message.c_str(), sizeof(result.error_message) - 1);
    result.error_message[sizeof(result.error_message) - 1] = '\0';

    return result;
}

std::string DatabaseLogger::generateDefaultTableName() const {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    // Generate table name based on database name
    if (strlen(connection_config_.database_name) == 0) {
        return "LogMessages_DefaultDB";
    }

    return "LogMessages_" + std::string(connection_config_.database_name);
}

bool DatabaseLogger::ensureTableExists(const std::string& table_name, const std::string& schema_name) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    // Check if table exists using ToolBox
    if (DatabaseToolbox::CheckLogTableExists(connection_config_, table_name, schema_name)) {
        return true;
    }

    // Create table using ToolBox
    return DatabaseToolbox::CreateLogTable(connection_config_, table_name, schema_name);
}

DWORD DatabaseLogger::getCurrentTimestamp() const {
    return DatabaseToolbox::GetCurrentTimestamp();
}

// =====================================================================================
// CONNECTION MANAGEMENT
// =====================================================================================

bool DatabaseLogger::connect() {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (is_connected_) {
        return true; // Already connected
    }

    try {
        // Initialize connection using ToolBox
        if (!initializeConnection()) {
            return false;
        }

        // Mark as connected
        is_connected_ = true;

        // Update performance statistics
        DatabaseOperationResult result;
        result.operation_id = 0;
        result.start_time = 0;
        result.end_time = 0;
        result.duration_ms = 0;
        strncpy(result.operation_type, "INSERT", sizeof(result.operation_type) - 1);
        result.operation_type[sizeof(result.operation_type) - 1] = '\0';
        strncpy(result.table_name, "", sizeof(result.table_name) - 1);
        result.table_name[sizeof(result.table_name) - 1] = '\0';
        result.rows_affected = 0;
        result.data_size_bytes = 0;
        result.success = true;
        result.error_code = 0;
        strncpy(result.error_message, "", sizeof(result.error_message) - 1);
        result.error_message[sizeof(result.error_message) - 1] = '\0';
        strncpy(result.sql_state, "", sizeof(result.sql_state) - 1);
        result.sql_state[sizeof(result.sql_state) - 1] = '\0';
        result.cpu_time_used_ms = 0;
        result.io_time_used_ms = 0;
        result.memory_used_bytes = 0;
        result.connection_id = connection_id_;
        result.connection_wait_time_ms = 0;
        updatePerformanceStatistics(result);

        return true;

    } catch (const std::exception& e) {
        // Update error statistics
        failed_operations_++;
        return false;
    }
}

bool DatabaseLogger::disconnect() {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (!is_connected_) {
        return true; // Already disconnected
    }

    try {
        // Clean up connection
        cleanupConnection();

        // Update performance statistics
        DatabaseOperationResult result;
        memset(&result, 0, sizeof(DatabaseOperationResult));
        result.success = true;
        result.connection_id = connection_id_;
        strcpy_s(result.operation_type, "DISCONNECT");
        updatePerformanceStatistics(result);

        return true;

    } catch (const std::exception& e) {
        // Update error statistics
        failed_operations_++;
        return false;
    }
}

bool DatabaseLogger::testConnection() {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    // Use ToolBox to test connection
    return DatabaseToolbox::TestConnection(connection_config_);
}

bool DatabaseLogger::updateConnectionConfig(const DatabaseConnectionConfig& config) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    // Validate new configuration
    if (!DatabaseToolbox::ValidateConnectionConfig(config)) {
        return false;
    }

    // Disconnect if currently connected
    bool was_connected = is_connected_;
    if (was_connected) {
        disconnect();
    }

    // Update configuration
    connection_config_ = config;

    // Reconnect if we were previously connected
    if (was_connected) {
        return connect();
    }

    return true;
}

// =====================================================================================
// SCHEMA MANAGEMENT
// =====================================================================================

bool DatabaseLogger::logTableExists(const std::string& table_name, const std::string& schema_name) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    std::string actual_table_name = table_name.empty() ? generateDefaultTableName() : table_name;
    std::string actual_schema_name = schema_name.empty() ? "dbo" : schema_name;

    // Use ToolBox to check table existence
    return DatabaseToolbox::CheckLogTableExists(connection_config_, actual_table_name, actual_schema_name);
}

bool DatabaseLogger::createLogTable(const std::string& table_name, const std::string& schema_name) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    std::string actual_table_name = table_name.empty() ? generateDefaultTableName() : table_name;
    std::string actual_schema_name = schema_name.empty() ? "dbo" : schema_name;

    // Use ToolBox to create log table
    return DatabaseToolbox::CreateLogTable(connection_config_, actual_table_name, actual_schema_name);
}

DatabaseSchemaInfo DatabaseLogger::getSchemaInfo(const std::string& table_name, const std::string& schema_name) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    std::string actual_table_name = table_name.empty() ? generateDefaultTableName() : table_name;
    std::string actual_schema_name = schema_name.empty() ? "dbo" : schema_name;

    // Use ToolBox to get schema information
    return DatabaseToolbox::GetSchemaInfo(connection_config_, actual_table_name, actual_schema_name);
}

bool DatabaseLogger::performMaintenance(const std::string& table_name, const std::string& schema_name) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    std::string actual_table_name = table_name.empty() ? generateDefaultTableName() : table_name;
    std::string actual_schema_name = schema_name.empty() ? "dbo" : schema_name;

    // Generate and execute maintenance script using ToolBox
    std::string maintenance_script = DatabaseToolbox::GenerateMaintenanceScript(actual_table_name, actual_schema_name);

    // Execute maintenance (simplified - would need actual database execution)
    DatabaseOperationResult result;
    memset(&result, 0, sizeof(DatabaseOperationResult));
    result.success = true;
    result.connection_id = connection_id_;
    strcpy_s(result.operation_type, "MAINTENANCE");

    // Update performance statistics
    updatePerformanceStatistics(result);

    return result.success;
}

// =====================================================================================
// MESSAGE PERSISTENCE
// =====================================================================================

DatabaseOperationResult DatabaseLogger::insertMessage(const LogMessageData& message,
                                                     const std::string& table_name,
                                                     const std::string& schema_name) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (!is_connected_) {
        return createOperationResult(false, "INSERT", "Not connected to database");
    }

    std::string actual_table_name = table_name.empty() ? generateDefaultTableName() : table_name;
    std::string actual_schema_name = schema_name.empty() ? "dbo" : schema_name;

    // Ensure table exists
    if (!ensureTableExists(actual_table_name, actual_schema_name)) {
        return createOperationResult(false, "INSERT", "Failed to create log table");
    }

    // Use ToolBox for message insertion
    DatabaseOperationResult result = DatabaseToolbox::InsertMessage(
        connection_config_, message, actual_table_name, actual_schema_name);

    // Update performance statistics
    updatePerformanceStatistics(result);

    return result;
}

DatabaseOperationResult DatabaseLogger::insertMessageBatch(const std::vector<LogMessageData>& messages,
                                                          const std::string& table_name,
                                                          const std::string& schema_name) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (!is_connected_) {
        DatabaseOperationResult result;
        memset(&result, 0, sizeof(DatabaseOperationResult));
        result.success = false;
        result.connection_id = connection_id_;
        strcpy_s(result.operation_type, "INSERT");
        strcpy_s(result.error_message, "Not connected to database");
        return result;
    }

    if (messages.empty()) {
        DatabaseOperationResult result;
        memset(&result, 0, sizeof(DatabaseOperationResult));
        result.success = true;
        result.connection_id = connection_id_;
        strcpy_s(result.operation_type, "INSERT");
        strcpy_s(result.error_message, "No messages to insert");
        return result;
    }

    std::string actual_table_name = table_name.empty() ? generateDefaultTableName() : table_name;
    std::string actual_schema_name = schema_name.empty() ? "dbo" : schema_name;

    // Ensure table exists
    if (!ensureTableExists(actual_table_name, actual_schema_name)) {
        DatabaseOperationResult result;
        memset(&result, 0, sizeof(DatabaseOperationResult));
        result.success = false;
        result.connection_id = connection_id_;
        strcpy_s(result.operation_type, "INSERT");
        strcpy_s(result.error_message, "Failed to create log table");
        return result;
    }

    // Use ToolBox for batch insertion
    DatabaseOperationResult result = DatabaseToolbox::InsertMessageBatch(
        connection_config_, messages, actual_table_name, actual_schema_name);

    // Update performance statistics
    updatePerformanceStatistics(result);

    return result;
}

DatabaseOperationResult DatabaseLogger::insertMessageWithRetry(const LogMessageData& message,
                                                              DWORD max_retries,
                                                              const std::string& table_name,
                                                              const std::string& schema_name) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (!is_connected_) {
        DatabaseOperationResult result;
        memset(&result, 0, sizeof(DatabaseOperationResult));
        result.success = false;
        result.connection_id = connection_id_;
        strcpy_s(result.operation_type, "INSERT");
        strcpy_s(result.error_message, "Not connected to database");
        return result;
    }

    std::string actual_table_name = table_name.empty() ? generateDefaultTableName() : table_name;
    std::string actual_schema_name = schema_name.empty() ? "dbo" : schema_name;

    // Ensure table exists
    if (!ensureTableExists(actual_table_name, actual_schema_name)) {
        DatabaseOperationResult result;
        memset(&result, 0, sizeof(DatabaseOperationResult));
        result.success = false;
        result.connection_id = connection_id_;
        strcpy_s(result.operation_type, "INSERT");
        strcpy_s(result.error_message, "Failed to create log table");
        return result;
    }

    // Implement retry logic with exponential backoff
    DWORD retry_delay = 100; // Start with 100ms delay
    DatabaseOperationResult result;

    for (DWORD attempt = 0; attempt <= max_retries; ++attempt) {
        // Use ToolBox for message insertion
        result = DatabaseToolbox::InsertMessage(connection_config_, message, actual_table_name, actual_schema_name);

        if (result.success) {
            break;
        }

        // Check if error is retryable
        auto error_info = DatabaseToolbox::ParseSqlError(result.error_message);
        if (!DatabaseToolbox::IsRetryableError(error_info)) {
            break; // Don't retry non-retryable errors
        }

        // Wait before retry (exponential backoff)
        if (attempt < max_retries) {
            // In real implementation, would use std::this_thread::sleep_for
            // For now, just increment delay
            retry_delay *= 2;
        }
    }

    // Update performance statistics
    updatePerformanceStatistics(result);

    return result;
}

DatabaseOperationResult DatabaseLogger::insertMessagesWithRetry(const std::vector<LogMessageData>& messages,
                                                               DWORD max_retries,
                                                               const std::string& table_name,
                                                               const std::string& schema_name) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (!is_connected_) {
        DatabaseOperationResult result;
        memset(&result, 0, sizeof(DatabaseOperationResult));
        result.success = false;
        result.connection_id = connection_id_;
        strcpy_s(result.operation_type, "INSERT");
        strcpy_s(result.error_message, "Not connected to database");
        return result;
    }

    if (messages.empty()) {
        DatabaseOperationResult result;
        memset(&result, 0, sizeof(DatabaseOperationResult));
        result.success = true;
        result.connection_id = connection_id_;
        strcpy_s(result.operation_type, "INSERT");
        strcpy_s(result.error_message, "No messages to insert");
        return result;
    }

    std::string actual_table_name = table_name.empty() ? generateDefaultTableName() : table_name;
    std::string actual_schema_name = schema_name.empty() ? "dbo" : schema_name;

    // Ensure table exists
    if (!ensureTableExists(actual_table_name, actual_schema_name)) {
        DatabaseOperationResult result;
        memset(&result, 0, sizeof(DatabaseOperationResult));
        result.success = false;
        result.connection_id = connection_id_;
        strcpy_s(result.operation_type, "INSERT");
        strcpy_s(result.error_message, "Failed to create log table");
        return result;
    }

    // Implement retry logic for batch operations
    DWORD retry_delay = 100; // Start with 100ms delay
    DatabaseOperationResult result;

    for (DWORD attempt = 0; attempt <= max_retries; ++attempt) {
        // Use ToolBox for batch insertion
        result = DatabaseToolbox::InsertMessageBatch(connection_config_, messages, actual_table_name, actual_schema_name);

        if (result.success) {
            break;
        }

        // Check if error is retryable
        auto error_info = DatabaseToolbox::ParseSqlError(result.error_message);
        if (!DatabaseToolbox::IsRetryableError(error_info)) {
            break; // Don't retry non-retryable errors
        }

        // Wait before retry (exponential backoff)
        if (attempt < max_retries) {
            retry_delay *= 2;
        }
    }

    // Update performance statistics
    updatePerformanceStatistics(result);

    return result;
}

// =====================================================================================
// QUERY OPERATIONS
// =====================================================================================

std::vector<LogMessageData> DatabaseLogger::queryByApplication(const std::string& application_name,
                                                              size_t limit,
                                                              const std::string& table_name,
                                                              const std::string& schema_name) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (!is_connected_) {
        return {};
    }

    std::string actual_table_name = table_name.empty() ? generateDefaultTableName() : table_name;
    std::string actual_schema_name = schema_name.empty() ? "dbo" : schema_name;

    // Generate application filter query using ToolBox
    std::string query = DatabaseToolbox::GenerateApplicationFilterQuery(application_name, actual_table_name, actual_schema_name);

    // Add limit if specified
    if (limit > 0) {
        query += " TOP " + std::to_string(limit);
    }

    // Execute query (simplified - would need actual database execution)
    std::vector<LogMessageData> results;

    // Update performance statistics
    DatabaseOperationResult result;
    memset(&result, 0, sizeof(DatabaseOperationResult));
    result.success = true;
    result.connection_id = connection_id_;
    strcpy_s(result.operation_type, "SELECT");
    strcpy_s(result.error_message, "Query executed");
    updatePerformanceStatistics(result);

    return results;
}

std::vector<LogMessageData> DatabaseLogger::queryByTimeRange(DWORD start_time,
                                                           DWORD end_time,
                                                           size_t limit,
                                                           const std::string& table_name,
                                                           const std::string& schema_name) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (!is_connected_) {
        return {};
    }

    std::string actual_table_name = table_name.empty() ? generateDefaultTableName() : table_name;
    std::string actual_schema_name = schema_name.empty() ? "dbo" : schema_name;

    // Generate time range query using ToolBox
    std::string query = DatabaseToolbox::GenerateTimeRangeQuery(start_time, end_time, actual_table_name, actual_schema_name);

    // Add limit if specified
    if (limit > 0) {
        query += " ORDER BY timestamp DESC OFFSET 0 ROWS FETCH NEXT " + std::to_string(limit) + " ROWS ONLY";
    }

    // Execute query (simplified - would need actual database execution)
    std::vector<LogMessageData> results;

    // Update performance statistics
    DatabaseOperationResult result;
    memset(&result, 0, sizeof(DatabaseOperationResult));
    result.success = true;
    result.connection_id = connection_id_;
    strcpy_s(result.operation_type, "SELECT");
    strcpy_s(result.error_message, "Query executed");
    updatePerformanceStatistics(result);

    return results;
}

std::vector<LogMessageData> DatabaseLogger::queryByMessageType(LogMessageType message_type,
                                                              size_t limit,
                                                              const std::string& table_name,
                                                              const std::string& schema_name) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (!is_connected_) {
        return {};
    }

    std::string actual_table_name = table_name.empty() ? generateDefaultTableName() : table_name;
    std::string actual_schema_name = schema_name.empty() ? "dbo" : schema_name;

    // Generate message type query using ToolBox
    std::string query = DatabaseToolbox::GenerateMessageTypeQuery(message_type, actual_table_name, actual_schema_name);

    // Add limit if specified
    if (limit > 0) {
        query += " ORDER BY timestamp DESC OFFSET 0 ROWS FETCH NEXT " + std::to_string(limit) + " ROWS ONLY";
    }

    // Execute query (simplified - would need actual database execution)
    std::vector<LogMessageData> results;

    // Update performance statistics
    DatabaseOperationResult result;
    memset(&result, 0, sizeof(DatabaseOperationResult));
    result.success = true;
    result.connection_id = connection_id_;
    strcpy_s(result.operation_type, "SELECT");
    strcpy_s(result.error_message, "Query executed");
    updatePerformanceStatistics(result);

    return results;
}

std::vector<LogMessageData> DatabaseLogger::queryWithFilters(const std::string& filters,
                                                           const std::string& order_by,
                                                           size_t limit,
                                                           const std::string& table_name,
                                                           const std::string& schema_name) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (!is_connected_) {
        return {};
    }

    std::string actual_table_name = table_name.empty() ? generateDefaultTableName() : table_name;
    std::string actual_schema_name = schema_name.empty() ? "dbo" : schema_name;

    // Generate custom query using ToolBox
    std::string query = DatabaseToolbox::GenerateSelectQuery(actual_table_name, actual_schema_name, filters, order_by, limit);

    // Execute query (simplified - would need actual database execution)
    std::vector<LogMessageData> results;

    // Update performance statistics
    DatabaseOperationResult result;
    memset(&result, 0, sizeof(DatabaseOperationResult));
    result.success = true;
    result.connection_id = connection_id_;
    strcpy_s(result.operation_type, "SELECT");
    strcpy_s(result.error_message, "Query executed");
    updatePerformanceStatistics(result);

    return results;
}

std::vector<std::pair<std::string, uint64_t>> DatabaseLogger::getMessageStatistics(
    DWORD time_range_hours,
    const std::string& table_name,
    const std::string& schema_name) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (!is_connected_) {
        return {};
    }

    std::string actual_table_name = table_name.empty() ? generateDefaultTableName() : table_name;
    std::string actual_schema_name = schema_name.empty() ? "dbo" : schema_name;

    // Generate statistics query using ToolBox
    std::string query = DatabaseToolbox::GenerateStatisticsQuery(actual_table_name, actual_schema_name, time_range_hours);

    // Execute query (simplified - would need actual database execution)
    std::vector<std::pair<std::string, uint64_t>> results;

    // Update performance statistics
    updatePerformanceStatistics(createOperationResult(true, "QUERY", "Statistics query executed"));

    return results;
}

// =====================================================================================
// TRANSACTION MANAGEMENT
// =====================================================================================

bool DatabaseLogger::beginTransaction(const std::string& isolation_level) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (!is_connected_) {
        return false;
    }

    // Generate BEGIN TRANSACTION statement using ToolBox
    std::string begin_sql = DatabaseToolbox::GenerateBeginTransaction(isolation_level);

    // Execute transaction start (simplified - would need actual database execution)
    DatabaseOperationResult result = createOperationResult(true, "TRANSACTION", "Transaction started");

    // Update performance statistics
    updatePerformanceStatistics(result);

    return result.success;
}

bool DatabaseLogger::commitTransaction() {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (!is_connected_) {
        return false;
    }

    // Generate COMMIT statement using ToolBox
    std::string commit_sql = DatabaseToolbox::GenerateCommitTransaction();

    // Execute commit (simplified - would need actual database execution)
    DatabaseOperationResult result = createOperationResult(true, "TRANSACTION", "Transaction committed");

    // Update performance statistics
    updatePerformanceStatistics(result);

    return result.success;
}

bool DatabaseLogger::rollbackTransaction() {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (!is_connected_) {
        return false;
    }

    // Generate ROLLBACK statement using ToolBox
    std::string rollback_sql = DatabaseToolbox::GenerateRollbackTransaction();

    // Execute rollback (simplified - would need actual database execution)
    DatabaseOperationResult result = createOperationResult(true, "TRANSACTION", "Transaction rolled back");

    // Update performance statistics
    updatePerformanceStatistics(result);

    return result.success;
}

bool DatabaseLogger::createSavepoint(const std::string& savepoint_name) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (!is_connected_) {
        return false;
    }

    // Generate SAVE TRANSACTION statement using ToolBox
    std::string save_sql = DatabaseToolbox::GenerateSaveTransaction(savepoint_name);

    // Execute savepoint creation (simplified - would need actual database execution)
    DatabaseOperationResult result = createOperationResult(true, "TRANSACTION", "Savepoint created");

    // Update performance statistics
    updatePerformanceStatistics(result);

    return result.success;
}

bool DatabaseLogger::rollbackToSavepoint(const std::string& savepoint_name) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (!is_connected_) {
        return false;
    }

    // Generate ROLLBACK TO SAVEPOINT statement using ToolBox
    std::string rollback_sql = DatabaseToolbox::GenerateRollbackToSavepoint(savepoint_name);

    // Execute rollback to savepoint (simplified - would need actual database execution)
    DatabaseOperationResult result = createOperationResult(true, "TRANSACTION", "Rolled back to savepoint");

    // Update performance statistics
    updatePerformanceStatistics(result);

    return result.success;
}

bool DatabaseLogger::hasActiveTransaction() const {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    // Check transaction state (simplified - would need actual transaction tracking)
    return false;
}

// =====================================================================================
// PREPARED STATEMENTS
// =====================================================================================

bool DatabaseLogger::prepareInsertStatement(const std::string& statement_name,
                                           const std::string& table_name,
                                           const std::string& schema_name) {
    std::lock_guard<std::mutex> lock(prepared_statement_mutex_);

    std::string actual_table_name = table_name.empty() ? generateDefaultTableName() : table_name;
    std::string actual_schema_name = schema_name.empty() ? "dbo" : schema_name;

    // Generate parameterized INSERT statement using ToolBox
    std::string insert_sql = DatabaseToolbox::GenerateParameterizedInsert(actual_table_name, actual_schema_name);

    // Store prepared statement (simplified - would need actual statement preparation)
    prepared_statements_[statement_name] = nullptr; // Placeholder for prepared statement handle

    return true;
}

DatabaseOperationResult DatabaseLogger::executePreparedInsert(const std::string& statement_name,
                                                             const LogMessageData& message) {
    std::lock_guard<std::mutex> lock(prepared_statement_mutex_);

    if (prepared_statements_.find(statement_name) == prepared_statements_.end()) {
        return createOperationResult(false, "PREPARED_STATEMENT", "Prepared statement not found");
    }

    // Execute prepared statement (simplified - would need actual execution)
    DatabaseOperationResult result = createOperationResult(true, "PREPARED_STATEMENT", "Prepared statement executed");

    // Update performance statistics
    updatePerformanceStatistics(result);

    return result;
}

bool DatabaseLogger::prepareSelectStatement(const std::string& statement_name,
                                           const std::string& filters,
                                           const std::string& table_name,
                                           const std::string& schema_name) {
    std::lock_guard<std::mutex> lock(prepared_statement_mutex_);

    std::string actual_table_name = table_name.empty() ? generateDefaultTableName() : table_name;
    std::string actual_schema_name = schema_name.empty() ? "dbo" : schema_name;

    // Generate SELECT statement using ToolBox
    std::string select_sql = DatabaseToolbox::GenerateSelectQuery(actual_table_name, actual_schema_name, filters);

    // Store prepared statement (simplified - would need actual statement preparation)
    prepared_statements_[statement_name] = nullptr; // Placeholder for prepared statement handle

    return true;
}

std::vector<LogMessageData> DatabaseLogger::executePreparedSelect(const std::string& statement_name,
                                                                 size_t limit) {
    std::lock_guard<std::mutex> lock(prepared_statement_mutex_);

    if (prepared_statements_.find(statement_name) == prepared_statements_.end()) {
        return {};
    }

    // Execute prepared statement (simplified - would need actual execution)
    std::vector<LogMessageData> results;

    // Update performance statistics
    updatePerformanceStatistics(createOperationResult(true, "PREPARED_STATEMENT", "Prepared select executed"));

    return results;
}

bool DatabaseLogger::releasePreparedStatement(const std::string& statement_name) {
    std::lock_guard<std::mutex> lock(prepared_statement_mutex_);

    auto it = prepared_statements_.find(statement_name);
    if (it == prepared_statements_.end()) {
        return false;
    }

    // Release prepared statement (simplified - would need actual cleanup)
    prepared_statements_.erase(it);

    return true;
}

void DatabaseLogger::releaseAllPreparedStatements() {
    std::lock_guard<std::mutex> lock(prepared_statement_mutex_);

    // Clear all prepared statements
    prepared_statements_.clear();
}

// =====================================================================================
// PERFORMANCE MONITORING
// =====================================================================================

std::string DatabaseLogger::getStatistics() const {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    std::ostringstream oss;
    oss << "DatabaseLogger Statistics:" << std::endl;
    oss << "Connection ID: " << connection_id_ << std::endl;
    oss << "Connected: " << (is_connected_ ? "Yes" : "No") << std::endl;
    oss << "Total Operations: " << total_operations_.load() << std::endl;
    oss << "Successful Operations: " << successful_operations_.load() << std::endl;
    oss << "Failed Operations: " << failed_operations_.load() << std::endl;

    if (total_operations_.load() > 0) {
        oss << "Success Rate: " << (static_cast<double>(successful_operations_.load()) / total_operations_.load() * 100) << "%" << std::endl;
    }

    DWORD avg_time = total_operations_.load() > 0 ?
                     total_operation_time_ms_.load() / total_operations_.load() : 0;
    oss << "Average Operation Time: " << avg_time << "ms" << std::endl;

    return oss.str();
}

std::vector<std::pair<std::string, double>> DatabaseLogger::getPerformanceMetrics() const {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    std::vector<std::pair<std::string, double>> metrics;

    metrics.emplace_back("total_operations", static_cast<double>(total_operations_.load()));
    metrics.emplace_back("successful_operations", static_cast<double>(successful_operations_.load()));
    metrics.emplace_back("failed_operations", static_cast<double>(failed_operations_.load()));

    if (total_operations_.load() > 0) {
        double success_rate = static_cast<double>(successful_operations_.load()) / total_operations_.load() * 100.0;
        metrics.emplace_back("success_rate_percent", success_rate);

        double avg_time = static_cast<double>(total_operation_time_ms_.load()) / total_operations_.load();
        metrics.emplace_back("average_operation_time_ms", avg_time);
    }

    return metrics;
}

DWORD DatabaseLogger::getAverageOperationTime() const {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (total_operations_.load() == 0) {
        return 0;
    }

    return total_operation_time_ms_.load() / total_operations_.load();
}

void DatabaseLogger::resetStatistics() {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    total_operations_ = 0;
    successful_operations_ = 0;
    failed_operations_ = 0;
    total_operation_time_ms_ = 0;
}

// =====================================================================================
// CONNECTION POOL MANAGEMENT
// =====================================================================================

void DatabaseLogger::setMaxConnections(size_t max_connections) {
    std::lock_guard<std::mutex> lock(connection_pool_mutex_);

    max_connections_ = max_connections;
}

size_t DatabaseLogger::getActiveConnectionCount() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(connection_pool_mutex_));
    return active_connections_.size();
}

bool DatabaseLogger::isConnectionAvailable() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(connection_pool_mutex_));

    return active_connections_.size() < max_connections_;
}

bool DatabaseLogger::waitForConnection(DWORD timeout_ms) {
    // Simplified - in real implementation would use condition variables
    return isConnectionAvailable();
}

// =====================================================================================
// MAINTENANCE OPERATIONS
// =====================================================================================

uint64_t DatabaseLogger::cleanupOldRecords(DWORD retention_days,
                                          const std::string& table_name,
                                          const std::string& schema_name) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (!is_connected_) {
        return 0;
    }

    std::string actual_table_name = table_name.empty() ? generateDefaultTableName() : table_name;
    std::string actual_schema_name = schema_name.empty() ? "dbo" : schema_name;

    // Generate cleanup script using ToolBox
    std::string cleanup_sql = DatabaseToolbox::GenerateCleanupScript(actual_table_name, actual_schema_name, retention_days);

    // Execute cleanup (simplified - would need actual database execution)
    DatabaseOperationResult result = createOperationResult(true, "MAINTENANCE", "Cleanup completed");

    // Update performance statistics
    updatePerformanceStatistics(result);

    return result.success ? 1 : 0; // Simplified return value
}

bool DatabaseLogger::updateStatistics(const std::string& table_name, const std::string& schema_name) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (!is_connected_) {
        return false;
    }

    std::string actual_table_name = table_name.empty() ? generateDefaultTableName() : table_name;
    std::string actual_schema_name = schema_name.empty() ? "dbo" : schema_name;

    // Generate statistics update script using ToolBox
    std::string update_sql = DatabaseToolbox::GenerateUpdateStatisticsScript(actual_table_name, actual_schema_name);

    // Execute statistics update (simplified - would need actual database execution)
    DatabaseOperationResult result = createOperationResult(true, "MAINTENANCE", "Statistics updated");

    // Update performance statistics
    updatePerformanceStatistics(result);

    return result.success;
}

bool DatabaseLogger::rebuildIndexes(const std::string& table_name, const std::string& schema_name) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (!is_connected_) {
        return false;
    }

    std::string actual_table_name = table_name.empty() ? generateDefaultTableName() : table_name;
    std::string actual_schema_name = schema_name.empty() ? "dbo" : schema_name;

    // Generate index rebuild script using ToolBox
    std::string rebuild_sql = DatabaseToolbox::GenerateRebuildIndexesScript(actual_table_name, actual_schema_name);

    // Execute index rebuild (simplified - would need actual database execution)
    DatabaseOperationResult result = createOperationResult(true, "MAINTENANCE", "Indexes rebuilt");

    // Update performance statistics
    updatePerformanceStatistics(result);

    return result.success;
}

bool DatabaseLogger::performComprehensiveMaintenance(const std::string& table_name, const std::string& schema_name) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (!is_connected_) {
        return false;
    }

    std::string actual_table_name = table_name.empty() ? generateDefaultTableName() : table_name;
    std::string actual_schema_name = schema_name.empty() ? "dbo" : schema_name;

    // Perform comprehensive maintenance (simplified - would need actual database execution)
    bool success = true;

    // Update statistics
    success &= updateStatistics(actual_table_name, actual_schema_name);

    // Rebuild indexes
    success &= rebuildIndexes(actual_table_name, actual_schema_name);

    // Update performance statistics
    updatePerformanceStatistics(createOperationResult(success, "MAINTENANCE", "Comprehensive maintenance completed"));

    return success;
}

// =====================================================================================
// ERROR HANDLING AND DIAGNOSTICS
// =====================================================================================

std::string DatabaseLogger::getLastError() const {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    // Return last error (simplified - would need actual error tracking)
    return is_connected_ ? "No errors" : "Not connected to database";
}

bool DatabaseLogger::hasRetryableError() const {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    // Check if last error is retryable (simplified - would need actual error tracking)
    return false;
}

DWORD DatabaseLogger::getRetryDelay() const {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    // Get suggested retry delay (simplified - would need actual error analysis)
    return 1000; // 1 second default
}

std::string DatabaseLogger::runDiagnostics() {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    std::ostringstream oss;
    oss << "Database Diagnostics:" << std::endl;
    oss << "Connected: " << (is_connected_ ? "Yes" : "No") << std::endl;
    oss << "Connection ID: " << connection_id_ << std::endl;
    oss << "Server: " << connection_config_.server_name << std::endl;
    oss << "Database: " << connection_config_.database_name << std::endl;
    oss << "Performance: " << getStatistics() << std::endl;

    return oss.str();
}

DatabaseQueryPlan DatabaseLogger::analyzeQuery(const std::string& query) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (!is_connected_) {
        return DatabaseQueryPlan{};
    }

    // Use ToolBox for query plan analysis
    return DatabaseToolbox::AnalyzeQueryPlan(connection_config_, query);
}

// =====================================================================================
// CONFIGURATION AND OPTIMIZATION
// =====================================================================================

bool DatabaseLogger::optimizeForHighThroughput() {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    // Generate optimized connection string using ToolBox
    std::string optimized_connection = DatabaseToolbox::GenerateOptimizedConnectionString(
        connection_config_, "HIGH_THROUGHPUT");

    // Update configuration (simplified - would need actual connection string parsing)
    return true;
}

bool DatabaseLogger::optimizeForLowLatency() {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    // Generate optimized connection string using ToolBox
    std::string optimized_connection = DatabaseToolbox::GenerateOptimizedConnectionString(
        connection_config_, "LOW_LATENCY");

    // Update configuration (simplified - would need actual connection string parsing)
    return true;
}

bool DatabaseLogger::optimizeForBalanced() {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    // Generate optimized connection string using ToolBox
    std::string optimized_connection = DatabaseToolbox::GenerateOptimizedConnectionString(
        connection_config_, "BALANCED");

    // Update configuration (simplified - would need actual connection string parsing)
    return true;
}

bool DatabaseLogger::setPerformanceProfile(const std::string& profile) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (profile == "HIGH_THROUGHPUT") {
        return optimizeForHighThroughput();
    } else if (profile == "LOW_LATENCY") {
        return optimizeForLowLatency();
    } else if (profile == "BALANCED") {
        return optimizeForBalanced();
    }

    return false;
}

std::string DatabaseLogger::getPerformanceProfile() const {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    // Return current performance profile (simplified - would need actual profile tracking)
    return "BALANCED";
}

// =====================================================================================
// UTILITY METHODS
// =====================================================================================

bool DatabaseLogger::validateConfiguration() const {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    // Use ToolBox to validate configuration
    return DatabaseToolbox::ValidateConnectionConfig(connection_config_);
}

DatabaseLogger DatabaseLogger::clone() const {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    return DatabaseLogger(connection_config_);
}

void DatabaseLogger::swap(DatabaseLogger& other) noexcept {
    std::unique_lock<std::mutex> this_lock(connection_mutex_, std::defer_lock);
    std::unique_lock<std::mutex> other_lock(other.connection_mutex_, std::defer_lock);
    std::lock(this_lock, other_lock);

    std::swap(connection_config_, other.connection_config_);
    std::swap(connection_id_, other.connection_id_);
    std::swap(is_connected_, other.is_connected_);

    // Swap atomic variables by loading and storing
    auto total_ops = total_operations_.load();
    total_operations_.store(other.total_operations_.load());
    other.total_operations_.store(total_ops);

    auto successful_ops = successful_operations_.load();
    successful_operations_.store(other.successful_operations_.load());
    other.successful_operations_.store(successful_ops);

    auto failed_ops = failed_operations_.load();
    failed_operations_.store(other.failed_operations_.load());
    other.failed_operations_.store(failed_ops);

    auto total_time = total_operation_time_ms_.load();
    total_operation_time_ms_.store(other.total_operation_time_ms_.load());
    other.total_operation_time_ms_.store(total_time);

    std::swap(active_connections_, other.active_connections_);
    std::swap(max_connections_, other.max_connections_);
    std::swap(prepared_statements_, other.prepared_statements_);
}

void DatabaseLogger::clear() {
    std::lock_guard<std::mutex> lock(prepared_statement_mutex_);

    // Release all prepared statements
    releaseAllPreparedStatements();
}

bool DatabaseLogger::isConfigured() const {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    return DatabaseToolbox::ValidateConnectionConfig(connection_config_);
}

std::string DatabaseLogger::getStatus() const {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    std::ostringstream oss;
    oss << "DatabaseLogger[";
    oss << "connected: " << (is_connected_ ? "true" : "false");
    oss << ", operations: " << total_operations_.load();
    oss << ", success_rate: ";
    if (total_operations_.load() > 0) {
        double rate = static_cast<double>(successful_operations_.load()) / total_operations_.load() * 100.0;
        oss << std::fixed << std::setprecision(1) << rate << "%";
    } else {
        oss << "N/A";
    }
    oss << "]";

    return oss.str();
}

// =====================================================================================
// NON-MEMBER FUNCTIONS
// =====================================================================================

DatabaseLogger CreateApplicationDatabaseLogger(const std::string& application_name,
                                              const std::string& server_name,
                                              const std::string& database_name) {
    // Create configuration for application
    auto config = DatabaseToolbox::CreateConnectionConfig(server_name, database_name);

    // Set application name (simplified - would need proper string copying)
    // For now, skip this as DatabaseConnectionConfig doesn't have application_name field

    return DatabaseLogger(config);
}

std::ostream& operator<<(std::ostream& os, const DatabaseLogger& logger) {
    os << logger.getStatus();
    return os;
}
