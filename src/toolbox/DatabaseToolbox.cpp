/**
 * ASFMLogger Database Toolbox Implementation
 *
 * Static methods for database operations following toolbox architecture.
 * Pure functions for SQL Server integration and data persistence.
 */

#include "DatabaseToolbox.hpp"
#include "../src/ASFMLoggerCore.hpp"
#include "asfmlogger/structs/LogDataStructures.hpp"
#include "asfmlogger/structs/DatabaseConfiguration.hpp"
#include <sstream>
#include <algorithm>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <Windows.h>
#include <fstream>

// Static variables
std::unordered_map<uint32_t, DatabaseConnectionConfig> DatabaseToolbox::connection_configs_;
std::unordered_map<uint32_t, DatabaseConnectionState> DatabaseToolbox::connection_states_;
std::unordered_map<std::string, uint32_t> DatabaseToolbox::application_connection_map_;

// =================================================================================
// CONNECTION MANAGEMENT
// =================================================================================

DatabaseConnectionConfig DatabaseToolbox::CreateConnectionConfig(
    const std::string& server_name,
    const std::string& database_name,
    const std::string& username,
    const std::string& password,
    bool use_windows_auth) {

    DatabaseConnectionConfig config;
    memset(&config, 0, sizeof(DatabaseConnectionConfig));

    config.connection_id = GenerateConnectionId();
    strcpy_s(config.server_name, server_name.c_str());
    strcpy_s(config.database_name, database_name.c_str());
    strcpy_s(config.username, username.c_str());
    strcpy_s(config.password, password.c_str());

    config.use_windows_authentication = use_windows_auth;
    config.connection_timeout_seconds = 30;
    config.command_timeout_seconds = 60;
    config.enable_connection_pooling = true;
    config.enable_mars = true;
    config.encrypt_connection = true;
    config.trust_server_certificate = false;
    config.max_pool_size = 100;
    config.min_pool_size = 5;
    config.retry_count = 3;
    config.retry_delay_ms = 1000;
    config.enable_connection_monitoring = true;
    config.monitoring_interval_seconds = 60;
    config.created_time = GetCurrentTimestamp();
    config.last_modified = config.created_time;
    strcpy_s(config.created_by, "DatabaseToolbox");
    config.is_active = true;

    return config;
}

std::string DatabaseToolbox::BuildConnectionString(const DatabaseConnectionConfig& config) {
    std::ostringstream oss;

    oss << "Server=" << config.server_name << ";";
    oss << "Database=" << config.database_name << ";";

    if (config.use_windows_authentication) {
        oss << "Integrated Security=True;";
    } else {
        oss << "User Id=" << config.username << ";";
        oss << "Password=" << config.password << ";";
    }

    if (config.encrypt_connection) {
        oss << "Encrypt=True;";
        if (config.trust_server_certificate) {
            oss << "TrustServerCertificate=True;";
        }
    }

    if (config.enable_connection_pooling) {
        oss << "Pooling=True;";
        oss << "Max Pool Size=" << config.max_pool_size << ";";
        oss << "Min Pool Size=" << config.min_pool_size << ";";
    }

    if (config.enable_mars) {
        oss << "MultipleActiveResultSets=True;";
    }

    oss << "Connection Timeout=" << config.connection_timeout_seconds << ";";

    return oss.str();
}

bool DatabaseToolbox::ValidateConnectionConfig(const DatabaseConnectionConfig& config) {
    return config.connection_id != 0 &&
           strlen(config.server_name) > 0 &&
           strlen(config.database_name) > 0 &&
           (config.use_windows_authentication || strlen(config.username) > 0) &&
           config.is_active;
}

bool DatabaseToolbox::TestConnection(const DatabaseConnectionConfig& config) {
    if (!ValidateConnectionConfig(config)) {
        return false;
    }

    // In a real implementation, this would test the actual database connection
    // For now, we'll simulate a successful connection test
    return true;
}

bool DatabaseToolbox::InitializeConnectionState(uint32_t connection_id, const DatabaseConnectionConfig& config) {
    DatabaseConnectionState state;
    memset(&state, 0, sizeof(DatabaseConnectionState));

    state.connection_id = connection_id;
    state.connection_start_time = GetCurrentTimestamp();
    state.last_activity_time = state.connection_start_time;
    state.is_connected = true;
    state.is_busy = false;

    connection_states_[connection_id] = state;
    return true;
}

// =================================================================================
// SCHEMA MANAGEMENT
// =================================================================================

std::string DatabaseToolbox::GenerateLogTableScript(const std::string& table_name, const std::string& schema_name) {
    std::ostringstream oss;

    oss << "IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'";
    if (!schema_name.empty() && schema_name != "dbo") {
        oss << schema_name << ".";
    }
    oss << table_name << "') AND type in (N'U'))" << std::endl;
    oss << "BEGIN" << std::endl;
    oss << "CREATE TABLE ";
    if (!schema_name.empty() && schema_name != "dbo") {
        oss << schema_name << ".";
    }
    oss << table_name << " (" << std::endl;
    oss << "    message_id bigint IDENTITY(1,1) PRIMARY KEY," << std::endl;
    oss << "    instance_id int NOT NULL," << std::endl;
    oss << "    message_type int NOT NULL," << std::endl;
    oss << "    timestamp_seconds bigint NOT NULL," << std::endl;
    oss << "    timestamp_microseconds int NOT NULL," << std::endl;
    oss << "    process_id int NOT NULL," << std::endl;
    oss << "    thread_id int NOT NULL," << std::endl;
    oss << "    message nvarchar(1024) NOT NULL," << std::endl;
    oss << "    component nvarchar(128) NOT NULL," << std::endl;
    oss << "    function_name nvarchar(128) NOT NULL," << std::endl;
    oss << "    source_file nvarchar(256) NOT NULL," << std::endl;
    oss << "    line_number int NOT NULL," << std::endl;
    oss << "    severity_string nvarchar(16) NOT NULL," << std::endl;
    oss << "    created_time datetime2 NOT NULL DEFAULT GETUTCDATE()" << std::endl;
    oss << ");" << std::endl;
    oss << "END;" << std::endl;

    return oss.str();
}

std::string DatabaseToolbox::GenerateLogIndexesScript(const std::string& table_name, const std::string& schema_name) {
    std::ostringstream oss;

    std::string schema_prefix = (!schema_name.empty() && schema_name != "dbo") ? schema_name + "." : "";

    oss << "IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'";
    oss << schema_prefix << table_name << "') AND name = N'IX_" << table_name << "_Timestamp')" << std::endl;
    oss << "BEGIN" << std::endl;
    oss << "CREATE INDEX IX_" << table_name << "_Timestamp ON ";
    oss << schema_prefix << table_name << " (timestamp_seconds DESC);" << std::endl;
    oss << "END;" << std::endl;

    oss << "IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'";
    oss << schema_prefix << table_name << "') AND name = N'IX_" << table_name << "_Type')" << std::endl;
    oss << "BEGIN" << std::endl;
    oss << "CREATE INDEX IX_" << table_name << "_Type ON ";
    oss << schema_prefix << table_name << " (message_type, timestamp_seconds DESC);" << std::endl;
    oss << "END;" << std::endl;

    oss << "IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'";
    oss << schema_prefix << table_name << "') AND name = N'IX_" << table_name << "_Instance')" << std::endl;
    oss << "BEGIN" << std::endl;
    oss << "CREATE INDEX IX_" << table_name << "_Instance ON ";
    oss << schema_prefix << table_name << " (instance_id, timestamp_seconds DESC);" << std::endl;
    oss << "END;" << std::endl;

    return oss.str();
}

std::string DatabaseToolbox::GenerateStoredProceduresScript(const std::string& table_name, const std::string& schema_name) {
    std::ostringstream oss;

    std::string schema_prefix = (!schema_name.empty() && schema_name != "dbo") ? schema_name + "." : "";

    // Insert procedure
    oss << "IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'";
    oss << schema_prefix << "InsertLogMessage') AND type in (N'P', N'PC'))" << std::endl;
    oss << "BEGIN" << std::endl;
    oss << "EXEC('CREATE PROCEDURE " << schema_prefix << "InsertLogMessage " << std::endl;
    oss << "@instance_id int, @message_type int, @timestamp_seconds bigint, " << std::endl;
    oss << "@timestamp_microseconds int, @process_id int, @thread_id int, " << std::endl;
    oss << "@message nvarchar(1024), @component nvarchar(128), @function_name nvarchar(128), " << std::endl;
    oss << "@source_file nvarchar(256), @line_number int, @severity_string nvarchar(16) " << std::endl;
    oss << "AS " << std::endl;
    oss << "INSERT INTO " << schema_prefix << table_name << " " << std::endl;
    oss << "(instance_id, message_type, timestamp_seconds, timestamp_microseconds, process_id, thread_id, " << std::endl;
    oss << "message, component, function_name, source_file, line_number, severity_string) " << std::endl;
    oss << "VALUES " << std::endl;
    oss << "(@instance_id, @message_type, @timestamp_seconds, @timestamp_microseconds, @process_id, @thread_id, " << std::endl;
    oss << "@message, @component, @function_name, @source_file, @line_number, @severity_string);" << std::endl;
    oss << "SELECT SCOPE_IDENTITY() as message_id;')" << std::endl;
    oss << "END;" << std::endl;

    return oss.str();
}

bool DatabaseToolbox::CheckLogTableExists(const DatabaseConnectionConfig& connection_config,
                                         const std::string& table_name,
                                         const std::string& schema_name) {
    // In a real implementation, this would query the database
    // For now, we'll simulate the check
    return true;
}

bool DatabaseToolbox::CreateLogTable(const DatabaseConnectionConfig& connection_config,
                                    const std::string& table_name,
                                    const std::string& schema_name) {
    if (!ValidateConnectionConfig(connection_config)) {
        return false;
    }

    // In a real implementation, this would execute the SQL scripts
    // For now, we'll simulate successful table creation
    return true;
}

DatabaseSchemaInfo DatabaseToolbox::GetSchemaInfo(const DatabaseConnectionConfig& connection_config,
                                                const std::string& table_name,
                                                const std::string& schema_name) {
    DatabaseSchemaInfo info;
    memset(&info, 0, sizeof(DatabaseSchemaInfo));

    info.schema_id = GenerateSchemaId();
    strcpy_s(info.table_name, table_name.c_str());
    strcpy_s(info.schema_name, schema_name.c_str());
    info.creation_time = GetCurrentTimestamp();
    info.column_count = 13;
    info.row_count_estimate = 1000000;
    info.table_size_bytes = 1024 * 1024 * 100; // 100MB estimate
    info.index_count = 3;

    strcpy_s(info.primary_key_columns, "message_id");
    strcpy_s(info.clustered_index_columns, "timestamp_seconds");

    return info;
}

// =================================================================================
// MESSAGE INSERTION
// =================================================================================

std::string DatabaseToolbox::GenerateInsertStatement(const LogMessageData& message,
                                                    const std::string& table_name,
                                                    const std::string& schema_name) {
    std::ostringstream oss;

    std::string schema_prefix = (!schema_name.empty() && schema_name != "dbo") ? schema_name + "." : "";

    oss << "INSERT INTO " << schema_prefix << table_name << " ";
    oss << "(instance_id, message_type, timestamp_seconds, timestamp_microseconds, process_id, thread_id, ";
    oss << "message, component, function_name, source_file, line_number, severity_string) ";
    oss << "VALUES ";
    oss << "(" << message.instance_id << ", ";
    oss << static_cast<int>(message.type) << ", ";
    oss << message.timestamp.seconds << ", ";
    oss << message.timestamp.microseconds << ", ";
    oss << message.process_id << ", ";
    oss << message.thread_id << ", ";
    oss << "N'" << EscapeSqlString(message.message) << "', ";
    oss << "N'" << EscapeSqlString(message.component) << "', ";
    oss << "N'" << EscapeSqlString(message.function) << "', ";
    oss << "N'" << EscapeSqlString(message.file) << "', ";
    oss << message.line_number << ", ";
    oss << "N'" << EscapeSqlString(message.severity_string) << "');";

    return oss.str();
}

std::string DatabaseToolbox::GenerateParameterizedInsert(const std::string& table_name,
                                                        const std::string& schema_name) {
    std::ostringstream oss;

    std::string schema_prefix = (!schema_name.empty() && schema_name != "dbo") ? schema_name + "." : "";

    oss << "INSERT INTO " << schema_prefix << table_name << " ";
    oss << "(instance_id, message_type, timestamp_seconds, timestamp_microseconds, process_id, thread_id, ";
    oss << "message, component, function_name, source_file, line_number, severity_string) ";
    oss << "VALUES ";
    oss << "(@instance_id, @message_type, @timestamp_seconds, @timestamp_microseconds, @process_id, @thread_id, ";
    oss << "@message, @component, @function_name, @source_file, @line_number, @severity_string);";

    return oss.str();
}

std::vector<std::string> DatabaseToolbox::MessageToParameterValues(const LogMessageData& message) {
    std::vector<std::string> values;

    values.push_back(std::to_string(message.instance_id));
    values.push_back(std::to_string(static_cast<int>(message.type)));
    values.push_back(std::to_string(message.timestamp.seconds));
    values.push_back(std::to_string(message.timestamp.microseconds));
    values.push_back(std::to_string(message.process_id));
    values.push_back(std::to_string(message.thread_id));
    values.push_back(EscapeSqlString(message.message));
    values.push_back(EscapeSqlString(message.component));
    values.push_back(EscapeSqlString(message.function));
    values.push_back(EscapeSqlString(message.file));
    values.push_back(std::to_string(message.line_number));
    values.push_back(EscapeSqlString(message.severity_string));

    return values;
}

std::vector<std::string> DatabaseToolbox::MessageToColumnValues(const LogMessageData& message) {
    return MessageToParameterValues(message);
}

DatabaseOperationResult DatabaseToolbox::InsertMessage(const DatabaseConnectionConfig& connection_config,
                                                     const LogMessageData& message,
                                                     const std::string& table_name,
                                                     const std::string& schema_name) {
    DatabaseOperationResult result;
    memset(&result, 0, sizeof(DatabaseOperationResult));

    result.operation_id = GenerateOperationId();
    result.start_time = GetCurrentTimestamp();
    result.end_time = result.start_time + 1; // Simulate 1ms operation
    result.duration_ms = 1;

    strcpy_s(result.operation_type, "INSERT");
    strcpy_s(result.table_name, table_name.c_str());
    result.rows_affected = 1;
    result.data_size_bytes = strlen(message.message) + strlen(message.component) + strlen(message.function) + strlen(message.file);

    result.success = true;
    result.error_code = 0;
    result.cpu_time_used_ms = 1;
    result.io_time_used_ms = 0;
    result.memory_used_bytes = 1024;
    result.connection_id = connection_config.connection_id;

    return result;
}

// =================================================================================
// BATCH INSERTION
// =================================================================================

std::string DatabaseToolbox::GenerateBatchInsertStatement(const std::vector<LogMessageData>& messages,
                                                         const std::string& table_name,
                                                         const std::string& schema_name) {
    std::ostringstream oss;

    std::string schema_prefix = (!schema_name.empty() && schema_name != "dbo") ? schema_name + "." : "";

    oss << "INSERT INTO " << schema_prefix << table_name << " ";
    oss << "(instance_id, message_type, timestamp_seconds, timestamp_microseconds, process_id, thread_id, ";
    oss << "message, component, function_name, source_file, line_number, severity_string) ";
    oss << "VALUES ";

    for (size_t i = 0; i < messages.size(); ++i) {
        if (i > 0) oss << ", ";

        const LogMessageData& msg = messages[i];
        oss << "(" << msg.instance_id << ", ";
        oss << static_cast<int>(msg.type) << ", ";
        oss << msg.timestamp.seconds << ", ";
        oss << msg.timestamp.microseconds << ", ";
        oss << msg.process_id << ", ";
        oss << msg.thread_id << ", ";
        oss << "N'" << EscapeSqlString(msg.message) << "', ";
        oss << "N'" << EscapeSqlString(msg.component) << "', ";
        oss << "N'" << EscapeSqlString(msg.function) << "', ";
        oss << "N'" << EscapeSqlString(msg.file) << "', ";
        oss << msg.line_number << ", ";
        oss << "N'" << EscapeSqlString(msg.severity_string) << "')";
    }

    oss << ";";

    return oss.str();
}

DatabaseOperationResult DatabaseToolbox::InsertMessageBatch(const DatabaseConnectionConfig& connection_config,
                                                          const std::vector<LogMessageData>& messages,
                                                          const std::string& table_name,
                                                          const std::string& schema_name) {
    DatabaseOperationResult result;
    memset(&result, 0, sizeof(DatabaseOperationResult));

    result.operation_id = GenerateOperationId();
    result.start_time = GetCurrentTimestamp();
    result.end_time = result.start_time + messages.size(); // Simulate operation time
    result.duration_ms = static_cast<DWORD>(messages.size());

    strcpy_s(result.operation_type, "BATCH_INSERT");
    strcpy_s(result.table_name, table_name.c_str());
    result.rows_affected = static_cast<uint64_t>(messages.size());

    size_t total_size = 0;
    for (const auto& msg : messages) {
        total_size += strlen(msg.message) + strlen(msg.component) + strlen(msg.function) + strlen(msg.file);
    }
    result.data_size_bytes = total_size;

    result.success = true;
    result.error_code = 0;
    result.cpu_time_used_ms = static_cast<DWORD>(messages.size());
    result.io_time_used_ms = 0;
    result.memory_used_bytes = total_size + 1024;
    result.connection_id = connection_config.connection_id;

    return result;
}

size_t DatabaseToolbox::CalculateOptimalBatchSize(size_t average_message_size,
                                                 size_t available_memory_mb,
                                                 DWORD network_latency_ms) {
    size_t available_bytes = available_memory_mb * 1024 * 1024;
    size_t max_batch_size = available_bytes / average_message_size;

    // Adjust for network latency - higher latency means smaller batches
    if (network_latency_ms > 100) {
        max_batch_size = max_batch_size / 2;
    }

    // Reasonable limits
    if (max_batch_size < 10) max_batch_size = 10;
    if (max_batch_size > 1000) max_batch_size = 1000;

    return max_batch_size;
}

// =================================================================================
// QUERY OPERATIONS
// =================================================================================

std::string DatabaseToolbox::GenerateSelectQuery(const std::string& table_name,
                                                const std::string& schema_name,
                                                const std::string& filters,
                                                const std::string& order_by,
                                                size_t limit) {
    std::ostringstream oss;

    std::string schema_prefix = (!schema_name.empty() && schema_name != "dbo") ? schema_name + "." : "";

    oss << "SELECT ";
    if (limit > 0) oss << "TOP " << limit << " ";
    oss << "* FROM " << schema_prefix << table_name;

    if (!filters.empty()) {
        oss << " WHERE " << filters;
    }

    if (!order_by.empty()) {
        oss << " ORDER BY " << order_by;
    } else {
        oss << " ORDER BY timestamp_seconds DESC";
    }

    return oss.str();
}

std::string DatabaseToolbox::GenerateApplicationFilterQuery(const std::string& application_name,
                                                           const std::string& table_name,
                                                           const std::string& schema_name) {
    std::ostringstream oss;

    std::string schema_prefix = (!schema_name.empty() && schema_name != "dbo") ? schema_name + "." : "";

    oss << "SELECT * FROM " << schema_prefix << table_name;
    oss << " WHERE instance_id IN (SELECT instance_id FROM LoggerInstances WHERE application_name = '";
    oss << EscapeSqlString(application_name) << "')";
    oss << " ORDER BY timestamp_seconds DESC";

    return oss.str();
}

std::string DatabaseToolbox::GenerateTimeRangeQuery(DWORD start_time,
                                                   DWORD end_time,
                                                   const std::string& table_name,
                                                   const std::string& schema_name) {
    std::ostringstream oss;

    std::string schema_prefix = (!schema_name.empty() && schema_name != "dbo") ? schema_name + "." : "";

    oss << "SELECT * FROM " << schema_prefix << table_name;
    oss << " WHERE timestamp_seconds BETWEEN " << start_time << " AND " << end_time;
    oss << " ORDER BY timestamp_seconds DESC";

    return oss.str();
}

std::string DatabaseToolbox::GenerateMessageTypeQuery(LogMessageType message_type,
                                                     const std::string& table_name,
                                                     const std::string& schema_name) {
    std::ostringstream oss;

    std::string schema_prefix = (!schema_name.empty() && schema_name != "dbo") ? schema_name + "." : "";

    oss << "SELECT * FROM " << schema_prefix << table_name;
    oss << " WHERE message_type = " << static_cast<int>(message_type);
    oss << " ORDER BY timestamp_seconds DESC";

    return oss.str();
}

std::string DatabaseToolbox::GenerateStatisticsQuery(const std::string& table_name,
                                                    const std::string& schema_name,
                                                    DWORD time_range_hours) {
    std::ostringstream oss;

    std::string schema_prefix = (!schema_name.empty() && schema_name != "dbo") ? schema_name + "." : "";

    oss << "SELECT " << std::endl;
    oss << "    COUNT(*) as total_messages," << std::endl;
    oss << "    COUNT(DISTINCT instance_id) as unique_instances," << std::endl;
    oss << "    COUNT(DISTINCT process_id) as unique_processes," << std::endl;
    oss << "    AVG(DATEDIFF(second, '1970-01-01', created_time)) as avg_timestamp," << std::endl;
    oss << "    MIN(created_time) as first_message," << std::endl;
    oss << "    MAX(created_time) as last_message" << std::endl;
    oss << "FROM " << schema_prefix << table_name;

    if (time_range_hours > 0) {
        oss << " WHERE created_time >= DATEADD(hour, -" << time_range_hours << ", GETUTCDATE())";
    }

    return oss.str();
}

// =================================================================================
// DATA CONVERSION
// =================================================================================

std::unordered_map<std::string, std::string> DatabaseToolbox::MessageToColumnMap(const LogMessageData& message) {
    std::unordered_map<std::string, std::string> column_map;

    column_map["instance_id"] = std::to_string(message.instance_id);
    column_map["message_type"] = std::to_string(static_cast<int>(message.type));
    column_map["timestamp_seconds"] = std::to_string(message.timestamp.seconds);
    column_map["timestamp_microseconds"] = std::to_string(message.timestamp.microseconds);
    column_map["process_id"] = std::to_string(message.process_id);
    column_map["thread_id"] = std::to_string(message.thread_id);
    column_map["message"] = EscapeSqlString(message.message);
    column_map["component"] = EscapeSqlString(message.component);
    column_map["function_name"] = EscapeSqlString(message.function);
    column_map["source_file"] = EscapeSqlString(message.file);
    column_map["line_number"] = std::to_string(message.line_number);
    column_map["severity_string"] = EscapeSqlString(message.severity_string);

    return column_map;
}

LogMessageData DatabaseToolbox::RowDataToMessage(const std::unordered_map<std::string, std::string>& row_data) {
    LogMessageData message;
    memset(&message, 0, sizeof(LogMessageData));

    // This would parse database row data back into LogMessageData
    // For now, return empty message
    return message;
}

std::string DatabaseToolbox::EscapeSqlString(const std::string& input) {
    std::string escaped = input;

    // Replace single quotes with double quotes for SQL
    size_t pos = 0;
    while ((pos = escaped.find('\'', pos)) != std::string::npos) {
        escaped.replace(pos, 1, "''");
        pos += 2;
    }

    return escaped;
}

std::string DatabaseToolbox::UnixTimestampToSqlDateTime(DWORD unix_timestamp) {
    time_t time = static_cast<time_t>(unix_timestamp);
    struct tm tm_time;
    gmtime_s(&tm_time, &time);

    std::ostringstream oss;
    oss << std::put_time(&tm_time, "%Y-%m-%d %H:%M:%S");

    return oss.str();
}

DWORD DatabaseToolbox::SqlDateTimeToUnixTimestamp(const std::string& sql_datetime) {
    struct tm tm_time = {};
    std::istringstream iss(sql_datetime);
    iss >> std::get_time(&tm_time, "%Y-%m-%d %H:%M:%S");

    time_t time = mktime(&tm_time);
    return static_cast<DWORD>(time);
}

// =================================================================================
// TRANSACTION MANAGEMENT
// =================================================================================

std::string DatabaseToolbox::GenerateBeginTransaction(const std::string& isolation_level) {
    std::ostringstream oss;

    oss << "BEGIN TRANSACTION";
    if (!isolation_level.empty()) {
        oss << " WITH ISOLATION LEVEL " << isolation_level;
    }
    oss << ";";

    return oss.str();
}

std::string DatabaseToolbox::GenerateCommitTransaction() {
    return "COMMIT TRANSACTION;";
}

std::string DatabaseToolbox::GenerateRollbackTransaction() {
    return "ROLLBACK TRANSACTION;";
}

std::string DatabaseToolbox::GenerateSaveTransaction(const std::string& savepoint_name) {
    return "SAVE TRANSACTION " + savepoint_name + ";";
}

std::string DatabaseToolbox::GenerateRollbackToSavepoint(const std::string& savepoint_name) {
    return "ROLLBACK TRANSACTION " + savepoint_name + ";";
}

// =================================================================================
// PERFORMANCE OPTIMIZATION
// =================================================================================

DatabaseQueryPlan DatabaseToolbox::AnalyzeQueryPlan(const DatabaseConnectionConfig& connection_config,
                                                  const std::string& query) {
    DatabaseQueryPlan plan;
    memset(&plan, 0, sizeof(DatabaseQueryPlan));

    plan.plan_id = GeneratePlanId();
    strcpy_s(plan.query_hash, "dummy_hash");
    plan.plan_creation_time = GetCurrentTimestamp();
    plan.estimated_cost = 1.0;
    plan.estimated_rows = 1000;
    plan.estimated_cpu_cost = 0.5;
    plan.estimated_io_cost = 0.5;
    plan.operation_count = 3;
    strcpy_s(plan.plan_type, "SELECT");
    plan.uses_parallel_execution = false;
    plan.degree_of_parallelism = 1;
    plan.indexes_used_count = 1;
    strcpy_s(plan.indexes_used[0], "IX_LogMessages_Timestamp");
    plan.requires_table_scan = false;
    plan.has_warnings = false;

    return plan;
}

std::vector<std::string> DatabaseToolbox::GenerateIndexOptimizationSuggestions(const DatabaseSchemaInfo& schema_info) {
    std::vector<std::string> suggestions;

    if (schema_info.fragmentation_percentage > 30) {
        suggestions.push_back("High index fragmentation detected - consider index rebuild");
    }

    if (schema_info.row_count_estimate > 1000000 && schema_info.index_count < 3) {
        suggestions.push_back("Large table with few indexes - consider adding covering indexes");
    }

    return suggestions;
}

size_t DatabaseToolbox::CalculateOptimalPoolSize(size_t expected_concurrent_operations,
                                                DWORD average_operation_time_ms,
                                                size_t system_memory_gb) {
    size_t base_pool_size = expected_concurrent_operations * 2;

    // Adjust for operation time - longer operations need larger pool
    if (average_operation_time_ms > 1000) {
        base_pool_size = static_cast<size_t>(base_pool_size * 1.5);
    }

    // Adjust for available memory
    size_t max_pool_size = system_memory_gb * 10; // Assume 10 connections per GB

    size_t optimal_size = (base_pool_size < max_pool_size) ? base_pool_size : max_pool_size;

    // Reasonable limits
    if (optimal_size < 5) optimal_size = 5;
    if (optimal_size > 200) optimal_size = 200;

    return optimal_size;
}

std::string DatabaseToolbox::GenerateOptimizedConnectionString(const DatabaseConnectionConfig& base_config,
                                                             const std::string& performance_profile) {
    std::string connection_string = BuildConnectionString(base_config);

    if (performance_profile == "HIGH_THROUGHPUT") {
        // Optimize for high throughput
        connection_string += "Connection Lifetime=300;Max Pool Size=200;";
    } else if (performance_profile == "LOW_LATENCY") {
        // Optimize for low latency
        connection_string += "Connection Lifetime=60;Max Pool Size=50;";
    }

    return connection_string;
}

// =================================================================================
// MAINTENANCE OPERATIONS
// =================================================================================

std::string DatabaseToolbox::GenerateMaintenanceScript(const std::string& table_name,
                                                      const std::string& schema_name) {
    std::ostringstream oss;

    std::string schema_prefix = (!schema_name.empty() && schema_name != "dbo") ? schema_name + "." : "";

    oss << "-- Update statistics" << std::endl;
    oss << "UPDATE STATISTICS " << schema_prefix << table_name << ";" << std::endl;

    oss << "-- Rebuild fragmented indexes" << std::endl;
    oss << "ALTER INDEX ALL ON " << schema_prefix << table_name << " REBUILD;" << std::endl;

    return oss.str();
}

std::string DatabaseToolbox::GenerateCleanupScript(const std::string& table_name,
                                                  const std::string& schema_name,
                                                  DWORD retention_days) {
    std::ostringstream oss;

    std::string schema_prefix = (!schema_name.empty() && schema_name != "dbo") ? schema_name + "." : "";

    oss << "DELETE FROM " << schema_prefix << table_name;
    oss << " WHERE created_time < DATEADD(day, -" << retention_days << ", GETUTCDATE());";

    return oss.str();
}

std::string DatabaseToolbox::GenerateUpdateStatisticsScript(const std::string& table_name,
                                                           const std::string& schema_name) {
    std::ostringstream oss;

    std::string schema_prefix = (!schema_name.empty() && schema_name != "dbo") ? schema_name + "." : "";

    oss << "UPDATE STATISTICS " << schema_prefix << table_name << " WITH FULLSCAN;";

    return oss.str();
}

std::string DatabaseToolbox::GenerateRebuildIndexesScript(const std::string& table_name,
                                                         const std::string& schema_name) {
    std::ostringstream oss;

    std::string schema_prefix = (!schema_name.empty() && schema_name != "dbo") ? schema_name + "." : "";

    oss << "ALTER INDEX ALL ON " << schema_prefix << table_name << " REBUILD WITH (ONLINE = ON);";

    return oss.str();
}

// =================================================================================
// ERROR HANDLING AND DIAGNOSTICS
// =================================================================================

std::unordered_map<std::string, std::string> DatabaseToolbox::ParseSqlError(const std::string& error_message) {
    std::unordered_map<std::string, std::string> parsed_error;

    parsed_error["raw_message"] = error_message;
    parsed_error["error_code"] = "0";
    parsed_error["severity"] = "UNKNOWN";
    parsed_error["state"] = "0";

    // Basic parsing - in real implementation would be more sophisticated
    if (error_message.find("connection") != std::string::npos) {
        parsed_error["category"] = "CONNECTION";
    } else if (error_message.find("timeout") != std::string::npos) {
        parsed_error["category"] = "TIMEOUT";
    } else {
        parsed_error["category"] = "GENERAL";
    }

    return parsed_error;
}

std::string DatabaseToolbox::GenerateUserFriendlyError(const std::unordered_map<std::string, std::string>& sql_error) {
    std::string category = sql_error.at("category");

    if (category == "CONNECTION") {
        return "Unable to connect to database. Please check connection settings and network connectivity.";
    } else if (category == "TIMEOUT") {
        return "Database operation timed out. Please try again or check database performance.";
    } else {
        return "A database error occurred. Please check the error details for more information.";
    }
}

bool DatabaseToolbox::IsRetryableError(const std::unordered_map<std::string, std::string>& sql_error) {
    std::string category = sql_error.at("category");

    return category == "TIMEOUT" || category == "CONNECTION";
}

DWORD DatabaseToolbox::GetRetryDelayForError(const std::unordered_map<std::string, std::string>& sql_error) {
    std::string category = sql_error.at("category");

    if (category == "TIMEOUT") {
        return 2000; // 2 seconds for timeout errors
    } else if (category == "CONNECTION") {
        return 1000; // 1 second for connection errors
    } else {
        return 500;  // 0.5 seconds for other errors
    }
}

std::string DatabaseToolbox::GenerateDiagnosticQuery(const std::string& table_name,
                                                    const std::string& schema_name) {
    std::ostringstream oss;

    std::string schema_prefix = (!schema_name.empty() && schema_name != "dbo") ? schema_name + "." : "";

    oss << "SELECT " << std::endl;
    oss << "    COUNT(*) as total_rows," << std::endl;
    oss << "    MIN(created_time) as oldest_message," << std::endl;
    oss << "    MAX(created_time) as newest_message," << std::endl;
    oss << "    DATEDIFF(day, MIN(created_time), MAX(created_time)) as date_range_days" << std::endl;
    oss << "FROM " << schema_prefix << table_name << ";" << std::endl;

    return oss.str();
}

// =================================================================================
// CONFIGURATION AND DEFAULTS
// =================================================================================

DatabaseConnectionConfig DatabaseToolbox::CreateDefaultLoggingConfig(const std::string& server_name,
                                                                    const std::string& database_name) {
    return CreateConnectionConfig(server_name, database_name, "", "", true);
}

DatabaseConnectionConfig DatabaseToolbox::CreateHighPerformanceConfig(const std::string& server_name,
                                                                     const std::string& database_name) {
    DatabaseConnectionConfig config = CreateConnectionConfig(server_name, database_name, "", "", true);

    config.max_pool_size = 200;
    config.min_pool_size = 10;
    config.connection_timeout_seconds = 60;
    config.command_timeout_seconds = 120;
    config.enable_connection_pooling = true;
    config.enable_mars = true;

    return config;
}

DatabaseConnectionConfig DatabaseToolbox::ParseConnectionString(const std::string& connection_string) {
    DatabaseConnectionConfig config;
    memset(&config, 0, sizeof(DatabaseConnectionConfig));

    // Basic parsing - in real implementation would be more sophisticated
    config.connection_id = GenerateConnectionId();
    strcpy_s(config.connection_string, connection_string.c_str());

    // Extract server name
    size_t server_pos = connection_string.find("Server=");
    if (server_pos != std::string::npos) {
        size_t server_end = connection_string.find(';', server_pos);
        if (server_end != std::string::npos) {
            std::string server_part = connection_string.substr(server_pos + 7, server_end - server_pos - 7);
            strcpy_s(config.server_name, server_part.c_str());
        }
    }

    // Extract database name
    size_t db_pos = connection_string.find("Database=");
    if (db_pos != std::string::npos) {
        size_t db_end = connection_string.find(';', db_pos);
        if (db_end != std::string::npos) {
            std::string db_part = connection_string.substr(db_pos + 9, db_end - db_pos - 9);
            strcpy_s(config.database_name, db_part.c_str());
        }
    }

    return config;
}

bool DatabaseToolbox::ValidateOperationResult(const DatabaseOperationResult& result) {
    return result.success && result.error_code == 0;
}

// =================================================================================
// UTILITY FUNCTIONS
// =================================================================================

std::string DatabaseToolbox::OperationResultToString(const DatabaseOperationResult& result) {
    std::ostringstream oss;

    oss << "Operation: " << result.operation_type << std::endl;
    oss << "Duration: " << result.duration_ms << "ms" << std::endl;
    oss << "Rows Affected: " << result.rows_affected << std::endl;
    oss << "Success: " << (result.success ? "Yes" : "No") << std::endl;

    if (!result.success) {
        oss << "Error: " << result.error_message << std::endl;
    }

    return oss.str();
}

std::string DatabaseToolbox::ConnectionConfigToString(const DatabaseConnectionConfig& config, bool mask_password) {
    std::ostringstream oss;

    oss << "Server: " << config.server_name << std::endl;
    oss << "Database: " << config.database_name << std::endl;
    oss << "Windows Auth: " << (config.use_windows_authentication ? "Yes" : "No") << std::endl;

    if (!config.use_windows_authentication && !mask_password) {
        oss << "Username: " << config.username << std::endl;
        oss << "Password: " << config.password << std::endl;
    }

    oss << "Connection Pooling: " << (config.enable_connection_pooling ? "Enabled" : "Disabled") << std::endl;
    oss << "Max Pool Size: " << config.max_pool_size << std::endl;

    return oss.str();
}

std::string DatabaseToolbox::ConnectionStateToString(const DatabaseConnectionState& state) {
    std::ostringstream oss;

    oss << "Connected: " << (state.is_connected ? "Yes" : "No") << std::endl;
    oss << "Busy: " << (state.is_busy ? "Yes" : "No") << std::endl;
    oss << "Total Commands: " << state.total_commands_executed << std::endl;
    oss << "Total Failures: " << state.total_failures << std::endl;
    oss << "Avg Response Time: " << state.average_response_time_ms << "ms" << std::endl;

    return oss.str();
}

DWORD DatabaseToolbox::GetCurrentTimestamp() {
    return static_cast<DWORD>(time(nullptr));
}

// =================================================================================
// PRIVATE HELPER METHODS
// =================================================================================

uint32_t DatabaseToolbox::GenerateConnectionId() {
    static uint32_t next_id = 1;
    return next_id++;
}

uint32_t DatabaseToolbox::GenerateOperationId() {
    static uint32_t next_id = 1;
    return next_id++;
}

uint32_t DatabaseToolbox::GenerateSchemaId() {
    static uint32_t next_id = 1;
    return next_id++;
}

uint32_t DatabaseToolbox::GeneratePlanId() {
    static uint32_t next_id = 1;
    return next_id++;
}

std::string DatabaseToolbox::FormatSqlDateTime(DWORD unix_timestamp) {
    return UnixTimestampToSqlDateTime(unix_timestamp);
}

DWORD DatabaseToolbox::ParseSqlDateTime(const std::string& sql_datetime) {
    return SqlDateTimeToUnixTimestamp(sql_datetime);
}

std::string DatabaseToolbox::GenerateColumnList(const std::string& table_name, const std::string& schema_name) {
    return "instance_id, message_type, timestamp_seconds, timestamp_microseconds, process_id, thread_id, message, component, function_name, source_file, line_number, severity_string";
}

std::string DatabaseToolbox::GenerateParameterList(size_t parameter_count) {
    std::ostringstream oss;

    for (size_t i = 0; i < parameter_count; ++i) {
        if (i > 0) oss << ", ";
        oss << "@param" << i;
    }

    return oss.str();
}

std::vector<std::string> DatabaseToolbox::ParseCommaSeparatedList(const std::string& input) {
    std::vector<std::string> parts;
    std::istringstream iss(input);
    std::string part;

    while (std::getline(iss, part, ',')) {
        parts.push_back(part);
    }

    return parts;
}

bool DatabaseToolbox::IsValidTableName(const std::string& table_name) {
    return !table_name.empty() && table_name.length() <= 128;
}

bool DatabaseToolbox::IsValidSchemaName(const std::string& schema_name) {
    return schema_name.empty() || schema_name.length() <= 128;
}

void DatabaseToolbox::InitializeDefaultConfigurations() {
    // Initialize default configurations if needed
}

bool DatabaseToolbox::IsInitialized() {
    return !connection_configs_.empty();
}