#ifndef __ASFM_LOGGER_CONFIGURATION_DATA_HPP__
#define __ASFM_LOGGER_CONFIGURATION_DATA_HPP__

/**
 * ASFMLogger Configuration System POD Structures
 *
 * Pure data structures for configuration management following toolbox architecture.
 * No methods, pure data for maximum compatibility and testability.
 */

#include "structs/LogDataStructures.hpp"
#include "structs/ImportanceConfiguration.hpp"
#include "structs/PersistencePolicy.hpp"
#include <cstdint>

// =====================================================================================
// CONFIGURATION DATA STRUCTURES
// =====================================================================================

/**
 * @brief POD structure for complete ASFMLogger configuration
 * Contains all configuration settings for the enhanced logging system
 */
struct ASFMLoggerConfiguration {
    uint32_t config_id;                        ///< Unique configuration identifier
    char config_name[128];                     ///< Human-readable configuration name
    char config_version[32];                   ///< Configuration version
    char description[512];                     ///< Configuration description

    // Core application settings
    char application_name[256];                ///< Application this config applies to
    char process_name[256];                    ///< Process name (optional)
    char environment[64];                      ///< Environment ("DEV", "TEST", "PROD")

    // Logging configuration
    bool console_output_enabled;                ///< Whether console output is enabled
    char log_file_path[512];                   ///< Path to log file
    size_t max_file_size_bytes;                ///< Maximum log file size
    size_t max_files_count;                    ///< Maximum number of log files to keep
    LogMessageType min_console_level;          ///< Minimum level for console output
    LogMessageType min_file_level;             ///< Minimum level for file output

    // Enhanced features configuration
    bool enhanced_features_enabled;             ///< Whether enhanced features are enabled
    bool database_logging_enabled;              ///< Whether database logging is enabled
    bool shared_memory_enabled;                 ///< Whether shared memory is enabled
    bool smart_queue_enabled;                   ///< Whether smart queuing is enabled

    // Database configuration
    char database_server[256];                  ///< SQL Server instance name
    char database_name[256];                    ///< Database name
    char database_username[128];                ///< Database username
    char database_password[256];                ///< Database password (encrypted)
    bool use_windows_authentication;             ///< Whether to use Windows authentication

    // Shared memory configuration
    char shared_memory_name[256];               ///< Shared memory mapping name
    size_t shared_memory_size_bytes;            ///< Size of shared memory region

    // Performance configuration
    size_t default_queue_size;                  ///< Default queue size for applications
    DWORD queue_flush_interval_ms;             ///< Queue flush interval
    bool enable_adaptive_persistence;          ///< Whether to adapt based on system conditions
    DWORD performance_monitoring_interval_ms;   ///< Performance monitoring frequency

    // Security and compliance
    bool enable_encryption;                     ///< Whether to encrypt log data
    char encryption_key[256];                   ///< Encryption key (if applicable)
    DWORD log_retention_days;                   ///< How long to retain logs
    bool enable_audit_trail;                    ///< Whether to enable audit trail

    // Monitoring and alerting
    bool enable_performance_monitoring;         ///< Whether to monitor performance
    bool enable_health_checks;                  ///< Whether to perform health checks
    DWORD health_check_interval_seconds;        ///< Health check frequency
    char alert_webhook_url[512];                ///< Webhook URL for alerts

    // Configuration metadata
    DWORD created_time;                         ///< When configuration was created
    DWORD last_modified;                        ///< When configuration was last modified
    char created_by[128];                       ///< Who created this configuration
    char modified_by[128];                      ///< Who last modified this configuration
    bool is_active;                             ///< Whether this configuration is active

    // Future extensibility
    char reserved[256];                         ///< Reserved for future use
};

/**
 * @brief POD structure for configuration validation result
 * Contains validation results and error information
 */
struct ConfigurationValidationResult {
    uint32_t config_id;                         ///< Configuration identifier
    bool is_valid;                              ///< Whether configuration is valid
    DWORD validation_time;                      ///< When validation was performed

    // Validation details
    size_t error_count;                         ///< Number of validation errors
    size_t warning_count;                       ///< Number of validation warnings
    char primary_error[512];                    ///< Primary validation error message

    // Section validation results
    bool application_settings_valid;            ///< Application settings section valid
    bool logging_settings_valid;                ///< Logging settings section valid
    bool database_settings_valid;               ///< Database settings section valid
    bool performance_settings_valid;            ///< Performance settings section valid
    bool security_settings_valid;               ///< Security settings section valid

    // Detailed error information
    struct ValidationError {
        char section[64];                       ///< Section where error occurred
        char field[128];                        ///< Field with error
        char error_message[256];                 ///< Error description
        char suggested_fix[256];                ///< Suggested fix for the error
    } errors[20];                               ///< Array of validation errors

    // Future extensibility
    char reserved[256];                         ///< Reserved for future use
};

/**
 * @brief POD structure for configuration deployment information
 * Tracks configuration deployment and rollback information
 */
struct ConfigurationDeploymentInfo {
    uint32_t deployment_id;                     ///< Unique deployment identifier
    uint32_t config_id;                         ///< Configuration being deployed
    char deployment_environment[64];            ///< Environment being deployed to
    char deployment_version[32];                ///< Version being deployed

    // Deployment details
    DWORD deployment_start_time;                ///< When deployment started
    DWORD deployment_end_time;                  ///< When deployment completed
    bool deployment_successful;                 ///< Whether deployment succeeded
    char deployed_by[128];                      ///< Who performed the deployment

    // Rollback information
    bool can_rollback;                          ///< Whether rollback is possible
    uint32_t previous_config_id;                ///< Previous configuration ID for rollback
    char rollback_reason[256];                  ///< Reason for rollback (if applicable)

    // Impact assessment
    size_t affected_applications_count;         ///< Number of applications affected
    size_t affected_instances_count;            ///< Number of instances affected
    char impact_level[32];                      ///< Impact level ("LOW", "MEDIUM", "HIGH", "CRITICAL")

    // Future extensibility
    char reserved[256];                         ///< Reserved for future use
};

/**
 * @brief POD structure for configuration template
 * Reusable configuration templates for different scenarios
 */
struct ConfigurationTemplate {
    uint32_t template_id;                       ///< Unique template identifier
    char template_name[128];                    ///< Template name
    char template_category[64];                 ///< Template category ("HIGH_PERFORMANCE", "COMPLIANCE", "DEBUG")
    char description[512];                      ///< Template description

    // Template configuration
    ASFMLoggerConfiguration base_configuration;  ///< Base configuration for this template
    char applicable_scenarios[10][128];          ///< Scenarios this template applies to
    size_t scenario_count;                      ///< Number of applicable scenarios

    // Template metadata
    char author[128];                           ///< Template author
    char version[32];                           ///< Template version
    DWORD creation_date;                        ///< When template was created
    bool is_official;                           ///< Whether this is an official template

    // Usage statistics
    uint64_t usage_count;                       ///< How many times template has been used
    DWORD last_used_date;                       ///< When template was last used
    double average_satisfaction_score;          ///< User satisfaction score (0-5)

    // Future extensibility
    char reserved[256];                         ///< Reserved for future use
};

/**
 * @brief POD structure for configuration change tracking
 * Tracks changes made to configurations for audit purposes
 */
struct ConfigurationChange {
    uint32_t change_id;                         ///< Unique change identifier
    uint32_t config_id;                         ///< Configuration that was changed
    char change_type[64];                       ///< Type of change ("CREATE", "UPDATE", "DELETE")
    char changed_by[128];                       ///< Who made the change

    // Change details
    DWORD change_time;                          ///< When change was made
    char change_description[512];               ///< Description of the change
    char section_changed[128];                  ///< Section of configuration that changed

    // Before/after values
    char value_before[1024];                    ///< Value before change
    char value_after[1024];                     ///< Value after change
    char change_justification[512];             ///< Justification for the change

    // Impact assessment
    char impact_level[32];                      ///< Impact level of the change
    bool requires_restart;                      ///< Whether change requires restart
    bool requires_testing;                      ///< Whether change requires testing

    // Future extensibility
    char reserved[256];                         ///< Reserved for future use
};

/**
 * @brief POD structure for configuration environment settings
 * Environment-specific configuration overrides
 */
struct ConfigurationEnvironmentSettings {
    uint32_t settings_id;                       ///< Unique settings identifier
    char environment_name[64];                  ///< Environment name ("DEV", "TEST", "PROD")
    char region[64];                            ///< Geographic region (optional)
    char data_center[128];                      ///< Data center identifier (optional)

    // Environment-specific overrides
    char log_level_override[32];                ///< Log level override for this environment
    char database_server_override[256];         ///< Database server override
    char performance_profile[64];               ///< Performance profile for this environment
    bool enable_debug_features;                 ///< Whether to enable debug features

    // Environment-specific limits
    size_t max_queue_size_override;             ///< Queue size override
    DWORD retention_days_override;              ///< Retention days override
    bool enable_compliance_mode;                ///< Whether compliance mode is enabled

    // Environment metadata
    char environment_owner[128];                ///< Environment owner
    char contact_information[256];              ///< Contact info for environment issues
    DWORD last_updated;                         ///< When settings were last updated

    // Future extensibility
    char reserved[256];                         ///< Reserved for future use
};

#endif // __ASFM_LOGGER_CONFIGURATION_DATA_HPP__