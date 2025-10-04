/**
 * ASFMLogger Configuration Toolbox Implementation
 *
 * Static methods for configuration management following toolbox architecture.
 * Pure functions for parsing, validation, and configuration processing.
 */

#include "ConfigurationToolbox.hpp"
#include "../ASFMLoggerCore.hpp"
#include "../structs/ConfigurationData.hpp"
#include "../structs/LogDataStructures.hpp"
#include "../structs/ImportanceConfiguration.hpp"
#include "../structs/PersistencePolicy.hpp"
#include "LoggerInstanceToolbox.hpp"

// Optional JSON support
#ifdef ASFMLOGGER_JSON_SUPPORT
#include <nlohmann/json.hpp>
#endif

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <cstdlib>

// Static variables
std::unordered_map<std::string, ASFMLoggerConfiguration> ConfigurationToolbox::application_configs_;
std::unordered_map<std::string, ConfigurationTemplate> ConfigurationToolbox::configuration_templates_;
std::vector<ConfigurationChange> ConfigurationToolbox::configuration_changes_;

// =====================================================================================
// CONFIGURATION PARSING
// =====================================================================================

ASFMLoggerConfiguration ConfigurationToolbox::ParseConfigurationFromJson(const std::string& json_config) {
#ifdef ASFMLOGGER_JSON_SUPPORT
    try {
        nlohmann::json json_obj = ParseJsonConfig(json_config);
        return JsonToConfiguration(json_obj);
    } catch (const std::exception&) {
        return CreateDefaultConfiguration("Unknown", "DEV");
    }
#else
    // JSON support not available, return default configuration
    return CreateDefaultConfiguration("Unknown", "DEV");
#endif
}

ASFMLoggerConfiguration ConfigurationToolbox::ParseConfigurationFromFile(const std::string& config_file) {
    std::ifstream file(config_file);
    if (!file.is_open()) {
        return CreateDefaultConfiguration("Unknown", "DEV");
    }

    std::string json_content((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
    file.close();

    return ParseConfigurationFromJson(json_content);
}

ASFMLoggerConfiguration ConfigurationToolbox::ParseConfigurationFromXml(const std::string& xml_config) {
    // Simplified XML parsing (in a real implementation, use a proper XML parser)
    ASFMLoggerConfiguration config = CreateDefaultConfiguration("Unknown", "DEV");

    // Basic XML parsing logic would go here
    // This is a placeholder implementation

    return config;
}

ASFMLoggerConfiguration ConfigurationToolbox::ParseConfigurationFromEnvironment(const std::string& application_name) {
    ASFMLoggerConfiguration config = CreateDefaultConfiguration(application_name, "PROD");

    // Parse environment variables
    std::string env = GetEnvironmentVariable("ASFM_LOGGER_ENVIRONMENT");
    if (!env.empty()) {
        strcpy_s(config.environment, env.c_str());
    }

    std::string log_level = GetEnvironmentVariable("ASFM_LOGGER_LEVEL");
    if (!log_level.empty()) {
        // Convert string to LogMessageType
        if (log_level == "DEBUG") config.min_console_level = LogMessageType::DEBUG;
        else if (log_level == "INFO") config.min_console_level = LogMessageType::INFO;
        else if (log_level == "WARN") config.min_console_level = LogMessageType::WARN;
        else if (log_level == "ERROR") config.min_console_level = LogMessageType::ERR;
    }

    std::string db_server = GetEnvironmentVariable("ASFM_LOGGER_DB_SERVER");
    if (!db_server.empty()) {
        strcpy_s(config.database_server, db_server.c_str());
    }

    return config;
}

ASFMLoggerConfiguration ConfigurationToolbox::ParseConfigurationFromCommandLine(int argc, char* argv[]) {
    ASFMLoggerConfiguration config = CreateDefaultConfiguration("CommandLineApp", "DEV");

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        std::vector<std::string> parts = ParseCommandLineArgument(arg);

        if (parts.size() >= 2) {
            std::string key = parts[0];
            std::string value = parts[1];

            if (key == "--environment" || key == "-env") {
                strcpy_s(config.environment, value.c_str());
            }
            else if (key == "--log-level" || key == "-l") {
                if (value == "DEBUG") config.min_console_level = LogMessageType::DEBUG;
                else if (value == "INFO") config.min_console_level = LogMessageType::INFO;
                else if (value == "WARN") config.min_console_level = LogMessageType::WARN;
                else if (value == "ERROR") config.min_console_level = LogMessageType::ERR;
            }
            else if (key == "--db-server") {
                strcpy_s(config.database_server, value.c_str());
            }
            else if (key == "--app-name") {
                strcpy_s(config.application_name, value.c_str());
            }
        }
    }

    return config;
}

// =====================================================================================
// CONFIGURATION VALIDATION
// =====================================================================================

ConfigurationValidationResult ConfigurationToolbox::ValidateConfiguration(const ASFMLoggerConfiguration& config) {
    ConfigurationValidationResult result;
    memset(&result, 0, sizeof(ConfigurationValidationResult));

    result.config_id = config.config_id;
    result.is_valid = true;
    result.validation_time = GetCurrentTimestamp();

    // Validate application settings
    result.application_settings_valid = ValidateApplicationSettings(config);
    if (!result.application_settings_valid) {
        result.is_valid = false;
        strcpy_s(result.primary_error, "Application settings validation failed");
    }

    // Validate logging settings
    result.logging_settings_valid = ValidateLoggingSettings(config);
    if (!result.logging_settings_valid) {
        result.is_valid = false;
        if (strlen(result.primary_error) == 0) {
            strcpy_s(result.primary_error, "Logging settings validation failed");
        }
    }

    // Validate database settings
    result.database_settings_valid = ValidateDatabaseSettings(config);
    if (!result.database_settings_valid) {
        result.is_valid = false;
        if (strlen(result.primary_error) == 0) {
            strcpy_s(result.primary_error, "Database settings validation failed");
        }
    }

    // Validate performance settings
    result.performance_settings_valid = ValidatePerformanceSettings(config);
    if (!result.performance_settings_valid) {
        result.is_valid = false;
        if (strlen(result.primary_error) == 0) {
            strcpy_s(result.primary_error, "Performance settings validation failed");
        }
    }

    // Validate security settings
    result.security_settings_valid = ValidateSecuritySettings(config);
    if (!result.security_settings_valid) {
        result.is_valid = false;
        if (strlen(result.primary_error) == 0) {
            strcpy_s(result.primary_error, "Security settings validation failed");
        }
    }

    return result;
}

bool ConfigurationToolbox::ValidateApplicationSettings(const ASFMLoggerConfiguration& config) {
    return config.config_id != 0 &&
           strlen(config.application_name) > 0 &&
           strlen(config.environment) > 0;
}

bool ConfigurationToolbox::ValidateLoggingSettings(const ASFMLoggerConfiguration& config) {
    return config.min_console_level >= LogMessageType::TRACE &&
           config.min_console_level <= LogMessageType::CRITICAL_LOG &&
           config.min_file_level >= LogMessageType::TRACE &&
           config.min_file_level <= LogMessageType::CRITICAL_LOG &&
           config.max_file_size_bytes > 0 &&
           config.max_files_count > 0;
}

bool ConfigurationToolbox::ValidateDatabaseSettings(const ASFMLoggerConfiguration& config) {
    if (!config.database_logging_enabled) {
        return true; // Database settings not required if disabled
    }

    return strlen(config.database_server) > 0 &&
           strlen(config.database_name) > 0;
}

bool ConfigurationToolbox::ValidatePerformanceSettings(const ASFMLoggerConfiguration& config) {
    return config.default_queue_size > 0 &&
           config.queue_flush_interval_ms > 0 &&
           config.performance_monitoring_interval_ms > 0;
}

bool ConfigurationToolbox::ValidateSecuritySettings(const ASFMLoggerConfiguration& config) {
    if (!config.enable_encryption) {
        return true; // Encryption settings not required if disabled
    }

    return strlen(config.encryption_key) > 0 &&
           config.log_retention_days > 0;
}

bool ConfigurationToolbox::QuickValidateConfiguration(const ASFMLoggerConfiguration& config) {
    return config.config_id != 0 &&
           strlen(config.application_name) > 0 &&
           strlen(config.environment) > 0 &&
           config.is_active;
}

// =====================================================================================
// CONFIGURATION SERIALIZATION
// =====================================================================================

std::string ConfigurationToolbox::ConfigurationToJson(const ASFMLoggerConfiguration& config, bool pretty_print) {
#ifdef ASFMLOGGER_JSON_SUPPORT
    nlohmann::json json_obj = ConfigurationToJson(config);

    if (pretty_print) {
        return json_obj.dump(4);
    } else {
        return json_obj.dump();
    }
#else
    // JSON support not available, return simple string representation
    return ConfigurationToString(config, false);
#endif
}

std::string ConfigurationToolbox::ConfigurationToXml(const ASFMLoggerConfiguration& config, bool pretty_print) {
    std::ostringstream oss;

    if (pretty_print) {
        oss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        oss << "<ASFMLoggerConfiguration>\n";
        oss << "  <ConfigId>" << config.config_id << "</ConfigId>\n";
        oss << "  <ConfigName>" << config.config_name << "</ConfigName>\n";
        oss << "  <ApplicationName>" << config.application_name << "</ApplicationName>\n";
        oss << "  <Environment>" << config.environment << "</Environment>\n";
        oss << "  <ConsoleOutputEnabled>" << (config.console_output_enabled ? "true" : "false") << "</ConsoleOutputEnabled>\n";
        oss << "  <LogFilePath>" << config.log_file_path << "</LogFilePath>\n";
        oss << "  <MinConsoleLevel>" << static_cast<int>(config.min_console_level) << "</MinConsoleLevel>\n";
        oss << "  <MinFileLevel>" << static_cast<int>(config.min_file_level) << "</MinFileLevel>\n";
        oss << "</ASFMLoggerConfiguration>";
    } else {
        oss << "<ASFMLoggerConfiguration><ConfigId>" << config.config_id << "</ConfigId><ConfigName>"
            << config.config_name << "</ConfigName><ApplicationName>" << config.application_name
            << "</ApplicationName><Environment>" << config.environment << "</Environment></ASFMLoggerConfiguration>";
    }

    return oss.str();
}

std::string ConfigurationToolbox::ConfigurationToEnvironmentVariables(const ASFMLoggerConfiguration& config) {
    std::ostringstream oss;

    oss << "ASFM_LOGGER_CONFIG_ID=" << config.config_id << std::endl;
    oss << "ASFM_LOGGER_APPLICATION_NAME=" << config.application_name << std::endl;
    oss << "ASFM_LOGGER_ENVIRONMENT=" << config.environment << std::endl;
    oss << "ASFM_LOGGER_CONSOLE_OUTPUT=" << (config.console_output_enabled ? "true" : "false") << std::endl;
    oss << "ASFM_LOGGER_LOG_LEVEL=" << static_cast<int>(config.min_console_level) << std::endl;

    if (strlen(config.log_file_path) > 0) {
        oss << "ASFM_LOGGER_LOG_FILE=" << config.log_file_path << std::endl;
    }

    return oss.str();
}

std::string ConfigurationToolbox::ConfigurationToCommandLine(const ASFMLoggerConfiguration& config) {
    std::ostringstream oss;

    oss << "--config-id " << config.config_id << " ";
    oss << "--application-name \"" << config.application_name << "\" ";
    oss << "--environment " << config.environment << " ";
    oss << "--console-output " << (config.console_output_enabled ? "true" : "false") << " ";
    oss << "--log-level " << static_cast<int>(config.min_console_level) << " ";

    if (strlen(config.log_file_path) > 0) {
        oss << "--log-file \"" << config.log_file_path << "\" ";
    }

    return oss.str();
}

bool ConfigurationToolbox::SaveConfigurationToFile(const ASFMLoggerConfiguration& config,
                                                  const std::string& file_path,
                                                  bool pretty_print) {
    std::ofstream file(file_path);
    if (!file.is_open()) {
        return false;
    }

    std::string json_content = ConfigurationToJson(config, pretty_print);
    file << json_content;
    file.close();

    return true;
}

// =====================================================================================
// DEFAULT CONFIGURATION CREATION
// =====================================================================================

ASFMLoggerConfiguration ConfigurationToolbox::CreateDefaultConfiguration(const std::string& application_name,
                                                                       const std::string& environment) {
    ASFMLoggerConfiguration config;
    memset(&config, 0, sizeof(ASFMLoggerConfiguration));

    config.config_id = GenerateConfigurationId();
    strcpy_s(config.config_name, "Default Configuration");
    strcpy_s(config.config_version, "1.0.0");
    strcpy_s(config.description, "Default ASFMLogger configuration");

    strcpy_s(config.application_name, application_name.c_str());
    strcpy_s(config.process_name, "");
    strcpy_s(config.environment, environment.c_str());

    // Logging configuration
    config.console_output_enabled = true;
    strcpy_s(config.log_file_path, "logs/asfm_logger.log");
    config.max_file_size_bytes = 10 * 1024 * 1024; // 10MB
    config.max_files_count = 5;
    config.min_console_level = LogMessageType::INFO;
    config.min_file_level = LogMessageType::DEBUG;

    // Enhanced features
    config.enhanced_features_enabled = true;
    config.database_logging_enabled = false;
    config.shared_memory_enabled = false;
    config.smart_queue_enabled = true;

    // Database configuration (empty for default)
    strcpy_s(config.database_server, "");
    strcpy_s(config.database_name, "");
    strcpy_s(config.database_username, "");
    strcpy_s(config.database_password, "");
    config.use_windows_authentication = true;

    // Shared memory configuration
    strcpy_s(config.shared_memory_name, "ASFMLoggerSharedMemory");
    config.shared_memory_size_bytes = 1024 * 1024; // 1MB

    // Performance configuration
    config.default_queue_size = 1000;
    config.queue_flush_interval_ms = 5000;
    config.enable_adaptive_persistence = true;
    config.performance_monitoring_interval_ms = 10000;

    // Security and compliance
    config.enable_encryption = false;
    strcpy_s(config.encryption_key, "");
    config.log_retention_days = 30;
    config.enable_audit_trail = false;

    // Monitoring and alerting
    config.enable_performance_monitoring = true;
    config.enable_health_checks = true;
    config.health_check_interval_seconds = 60;
    strcpy_s(config.alert_webhook_url, "");

    // Metadata
    config.created_time = GetCurrentTimestamp();
    config.last_modified = config.created_time;
    strcpy_s(config.created_by, "System");
    strcpy_s(config.modified_by, "System");
    config.is_active = true;

    return config;
}

ASFMLoggerConfiguration ConfigurationToolbox::CreateHighPerformanceConfiguration(const std::string& application_name,
                                                                               const std::string& environment) {
    ASFMLoggerConfiguration config = CreateDefaultConfiguration(application_name, environment);

    strcpy_s(config.config_name, "High Performance Configuration");
    strcpy_s(config.description, "Optimized for high-performance scenarios");

    // Optimize for performance
    config.min_console_level = LogMessageType::WARN;
    config.min_file_level = LogMessageType::WARN;
    config.max_file_size_bytes = 50 * 1024 * 1024; // 50MB
    config.max_files_count = 3;

    config.default_queue_size = 5000;
    config.queue_flush_interval_ms = 1000;
    config.performance_monitoring_interval_ms = 5000;

    // Batch persistence settings would go here if available in the struct

    return config;
}

ASFMLoggerConfiguration ConfigurationToolbox::CreateComprehensiveConfiguration(const std::string& application_name,
                                                                            const std::string& environment) {
    ASFMLoggerConfiguration config = CreateDefaultConfiguration(application_name, environment);

    strcpy_s(config.config_name, "Comprehensive Configuration");
    strcpy_s(config.description, "Comprehensive logging for detailed analysis");

    // Enable all features for comprehensive logging
    config.min_console_level = LogMessageType::DEBUG;
    config.min_file_level = LogMessageType::TRACE;
    config.max_file_size_bytes = 100 * 1024 * 1024; // 100MB
    config.max_files_count = 10;

    config.enhanced_features_enabled = true;
    config.database_logging_enabled = true;
    config.shared_memory_enabled = true;
    config.smart_queue_enabled = true;

    // Database configuration for comprehensive logging
    strcpy_s(config.database_server, "localhost");
    strcpy_s(config.database_name, "ASFMLogger");
    strcpy_s(config.database_username, "asfm_user");
    strcpy_s(config.database_password, "encrypted_password");

    config.enable_encryption = true;
    strcpy_s(config.encryption_key, "comprehensive_logging_key");
    config.log_retention_days = 90;
    config.enable_audit_trail = true;

    return config;
}

ASFMLoggerConfiguration ConfigurationToolbox::CreateDevelopmentConfiguration(const std::string& application_name) {
    ASFMLoggerConfiguration config = CreateDefaultConfiguration(application_name, "DEV");

    strcpy_s(config.config_name, "Development Configuration");
    strcpy_s(config.description, "Development-optimized configuration");

    // Development-friendly settings
    config.console_output_enabled = true;
    config.min_console_level = LogMessageType::TRACE;
    config.min_file_level = LogMessageType::TRACE;

    strcpy_s(config.log_file_path, "logs/dev_asfm_logger.log");

    config.enable_performance_monitoring = true;
    config.enable_health_checks = false; // Disable in development

    return config;
}

ASFMLoggerConfiguration ConfigurationToolbox::CreateProductionConfiguration(const std::string& application_name) {
    ASFMLoggerConfiguration config = CreateDefaultConfiguration(application_name, "PROD");

    strcpy_s(config.config_name, "Production Configuration");
    strcpy_s(config.description, "Production-optimized configuration");

    // Production-ready settings
    config.min_console_level = LogMessageType::INFO;
    config.min_file_level = LogMessageType::INFO;

    strcpy_s(config.log_file_path, "/var/log/asfm_logger/asfm_logger.log");

    config.enable_performance_monitoring = true;
    config.enable_health_checks = true;
    config.health_check_interval_seconds = 30;

    strcpy_s(config.alert_webhook_url, "https://monitoring.company.com/alerts");

    return config;
}

// =====================================================================================
// CONFIGURATION MERGING AND OVERRIDES
// =====================================================================================

ASFMLoggerConfiguration ConfigurationToolbox::MergeConfigurations(const ASFMLoggerConfiguration& base_config,
                                                                const ASFMLoggerConfiguration& override_config) {
    ASFMLoggerConfiguration merged = base_config;

    // Apply overrides only for non-empty fields
    if (strlen(override_config.application_name) > 0) {
        strcpy_s(merged.application_name, override_config.application_name);
    }

    if (strlen(override_config.environment) > 0) {
        strcpy_s(merged.environment, override_config.environment);
    }

    if (strlen(override_config.log_file_path) > 0) {
        strcpy_s(merged.log_file_path, override_config.log_file_path);
    }

    // Apply numeric overrides if they're non-zero
    if (override_config.max_file_size_bytes > 0) {
        merged.max_file_size_bytes = override_config.max_file_size_bytes;
    }

    if (override_config.max_files_count > 0) {
        merged.max_files_count = override_config.max_files_count;
    }

    if (override_config.min_console_level != LogMessageType::TRACE) {
        merged.min_console_level = override_config.min_console_level;
    }

    if (override_config.min_file_level != LogMessageType::TRACE) {
        merged.min_file_level = override_config.min_file_level;
    }

    // Apply boolean overrides
    merged.console_output_enabled = override_config.console_output_enabled;
    merged.enhanced_features_enabled = override_config.enhanced_features_enabled;
    merged.database_logging_enabled = override_config.database_logging_enabled;

    merged.last_modified = GetCurrentTimestamp();
    strcpy_s(merged.modified_by, "ConfigurationToolbox");

    return merged;
}

ASFMLoggerConfiguration ConfigurationToolbox::ApplyEnvironmentOverrides(const ASFMLoggerConfiguration& base_config,
                                                                       const ConfigurationEnvironmentSettings& environment_settings) {
    ASFMLoggerConfiguration config = base_config;

    if (strlen(environment_settings.log_level_override) > 0) {
        std::string level = environment_settings.log_level_override;
        if (level == "DEBUG") config.min_console_level = LogMessageType::DEBUG;
        else if (level == "INFO") config.min_console_level = LogMessageType::INFO;
        else if (level == "WARN") config.min_console_level = LogMessageType::WARN;
        else if (level == "ERROR") config.min_console_level = LogMessageType::ERR;
    }

    if (strlen(environment_settings.database_server_override) > 0) {
        strcpy_s(config.database_server, environment_settings.database_server_override);
    }

    if (strlen(environment_settings.performance_profile) > 0) {
        std::string profile = environment_settings.performance_profile;
        if (profile == "HIGH_PERFORMANCE") {
            config = CreateHighPerformanceConfiguration(config.application_name, config.environment);
        }
    }

    // Debug and compliance features would be set here if available in the struct

    if (environment_settings.max_queue_size_override > 0) {
        config.default_queue_size = environment_settings.max_queue_size_override;
    }

    if (environment_settings.retention_days_override > 0) {
        config.log_retention_days = environment_settings.retention_days_override;
    }

    return config;
}

ASFMLoggerConfiguration ConfigurationToolbox::ApplyApplicationOverrides(const ASFMLoggerConfiguration& base_config,
                                                                       const std::string& application_name) {
    ASFMLoggerConfiguration config = base_config;

    // Apply application-specific overrides
    if (application_name.find("Test") != std::string::npos) {
        config.min_console_level = LogMessageType::DEBUG;
        config.min_file_level = LogMessageType::DEBUG;
        config.enable_performance_monitoring = false;
    }

    if (application_name.find("Service") != std::string::npos) {
        config.min_console_level = LogMessageType::WARN;
        config.min_file_level = LogMessageType::INFO;
        config.enable_health_checks = true;
    }

    return config;
}

std::vector<std::string> ConfigurationToolbox::ExtractConfigurationDifferences(const ASFMLoggerConfiguration& config1,
                                                                              const ASFMLoggerConfiguration& config2) {
    std::vector<std::string> differences;

    if (strcmp(config1.application_name, config2.application_name) != 0) {
        differences.push_back("Application name differs");
    }

    if (strcmp(config1.environment, config2.environment) != 0) {
        differences.push_back("Environment differs");
    }

    if (config1.min_console_level != config2.min_console_level) {
        differences.push_back("Console log level differs");
    }

    if (config1.min_file_level != config2.min_file_level) {
        differences.push_back("File log level differs");
    }

    if (config1.database_logging_enabled != config2.database_logging_enabled) {
        differences.push_back("Database logging enabled differs");
    }

    return differences;
}

// =====================================================================================
// TEMPLATE MANAGEMENT
// =====================================================================================

ASFMLoggerConfiguration ConfigurationToolbox::CreateFromTemplate(const std::string& template_name,
                                                               const std::string& application_name,
                                                               const std::unordered_map<std::string, std::string>& customizations) {
    auto it = configuration_templates_.find(template_name);
    if (it == configuration_templates_.end()) {
        return CreateDefaultConfiguration(application_name, "DEV");
    }

    ASFMLoggerConfiguration config = it->second.base_configuration;
    strcpy_s(config.application_name, application_name.c_str());

    // Apply customizations
    for (const auto& pair : customizations) {
        if (pair.first == "environment") {
            strcpy_s(config.environment, pair.second.c_str());
        }
        else if (pair.first == "log_level") {
            if (pair.second == "DEBUG") config.min_console_level = LogMessageType::DEBUG;
            else if (pair.second == "INFO") config.min_console_level = LogMessageType::INFO;
        }
    }

    return config;
}

bool ConfigurationToolbox::SaveAsTemplate(const ASFMLoggerConfiguration& config,
                                         const std::string& template_name,
                                         const std::string& category) {
    ConfigurationTemplate config_template;
    memset(&config_template, 0, sizeof(ConfigurationTemplate));

    config_template.template_id = GenerateConfigurationId();
    strcpy_s(config_template.template_name, template_name.c_str());
    strcpy_s(config_template.template_category, category.c_str());
    strcpy_s(config_template.description, "User-created template");
    strcpy_s(config_template.author, "User");
    strcpy_s(config_template.version, "1.0.0");
    config_template.creation_date = GetCurrentTimestamp();
    config_template.is_official = false;

    configuration_templates_[template_name] = config_template;
    return true;
}

std::vector<std::string> ConfigurationToolbox::GetAvailableTemplates() {
    std::vector<std::string> templates;
    for (const auto& pair : configuration_templates_) {
        templates.push_back(pair.first);
    }
    return templates;
}

std::vector<std::string> ConfigurationToolbox::GetTemplatesByCategory(const std::string& category) {
    std::vector<std::string> templates;
    for (const auto& pair : configuration_templates_) {
        if (strcmp(pair.second.template_category, category.c_str()) == 0) {
            templates.push_back(pair.first);
        }
    }
    return templates;
}

bool ConfigurationToolbox::ValidateTemplate(const ASFMLoggerConfiguration& config) {
    return QuickValidateConfiguration(config);
}

// =====================================================================================
// CONFIGURATION CHANGE TRACKING
// =====================================================================================

bool ConfigurationToolbox::TrackConfigurationChange(const ASFMLoggerConfiguration& old_config,
                                                   const ASFMLoggerConfiguration& new_config,
                                                   const std::string& change_description,
                                                   const std::string& changed_by) {
    ConfigurationChange change;
    memset(&change, 0, sizeof(ConfigurationChange));

    change.change_id = GenerateConfigurationId();
    change.config_id = new_config.config_id;
    strcpy_s(change.change_type, "UPDATE");
    strcpy_s(change.changed_by, changed_by.c_str());
    strcpy_s(change.change_description, change_description.c_str());
    strcpy_s(change.section_changed, "GENERAL");

    change.change_time = GetCurrentTimestamp();

    // Store before/after values
    std::string before = ConfigurationToString(old_config, false);
    std::string after = ConfigurationToString(new_config, false);

    strcpy_s(change.value_before, before.substr(0, sizeof(change.value_before) - 1).c_str());
    strcpy_s(change.value_after, after.substr(0, sizeof(change.value_after) - 1).c_str());

    configuration_changes_.push_back(change);
    return true;
}

std::vector<ConfigurationChange> ConfigurationToolbox::GetConfigurationChangeHistory(uint32_t config_id,
                                                                                   size_t max_entries) {
    std::vector<ConfigurationChange> history;

    for (const auto& change : configuration_changes_) {
        if (change.config_id == config_id) {
            history.push_back(change);
            if (history.size() >= max_entries) {
                break;
            }
        }
    }

    return history;
}

std::vector<ConfigurationChange> ConfigurationToolbox::GetConfigurationChangesByUser(const std::string& changed_by,
                                                                                   size_t max_entries) {
    std::vector<ConfigurationChange> changes;

    for (const auto& change : configuration_changes_) {
        if (strcmp(change.changed_by, changed_by.c_str()) == 0) {
            changes.push_back(change);
            if (changes.size() >= max_entries) {
                break;
            }
        }
    }

    return changes;
}

std::vector<ConfigurationChange> ConfigurationToolbox::GetConfigurationChangesInTimeRange(DWORD start_time,
                                                                                        DWORD end_time) {
    std::vector<ConfigurationChange> changes;

    for (const auto& change : configuration_changes_) {
        if (change.change_time >= start_time && change.change_time <= end_time) {
            changes.push_back(change);
        }
    }

    return changes;
}

// =====================================================================================
// ENVIRONMENT-SPECIFIC CONFIGURATION
// =====================================================================================

ConfigurationEnvironmentSettings ConfigurationToolbox::LoadEnvironmentSettings(const std::string& environment,
                                                                              const std::string& region) {
    ConfigurationEnvironmentSettings settings;
    memset(&settings, 0, sizeof(ConfigurationEnvironmentSettings));

    settings.settings_id = GenerateConfigurationId();
    strcpy_s(settings.environment_name, environment.c_str());
    strcpy_s(settings.region, region.c_str());

    // Load environment-specific defaults
    if (environment == "PROD") {
        strcpy_s(settings.log_level_override, "INFO");
        strcpy_s(settings.performance_profile, "HIGH_PERFORMANCE");
        settings.enable_compliance_mode = true;
        settings.max_queue_size_override = 5000;
        settings.retention_days_override = 90;
    }
    else if (environment == "DEV") {
        strcpy_s(settings.log_level_override, "DEBUG");
        strcpy_s(settings.performance_profile, "DEVELOPMENT");
        settings.enable_debug_features = true;
        settings.max_queue_size_override = 1000;
        settings.retention_days_override = 7;
    }

    return settings;
}

bool ConfigurationToolbox::SaveEnvironmentSettings(const ConfigurationEnvironmentSettings& environment_settings) {
    // In a real implementation, save to persistent storage
    return true;
}

ASFMLoggerConfiguration ConfigurationToolbox::GetEnvironmentConfiguration(const ASFMLoggerConfiguration& base_config,
                                                                         const std::string& environment,
                                                                         const std::string& region) {
    ConfigurationEnvironmentSettings env_settings = LoadEnvironmentSettings(environment, region);
    return ApplyEnvironmentOverrides(base_config, env_settings);
}

bool ConfigurationToolbox::ValidateEnvironmentSettings(const ConfigurationEnvironmentSettings& settings) {
    return settings.settings_id != 0 &&
           strlen(settings.environment_name) > 0;
}

// =====================================================================================
// CONFIGURATION DEPLOYMENT
// =====================================================================================

ASFMLoggerConfiguration ConfigurationToolbox::PrepareForDeployment(const ASFMLoggerConfiguration& config,
                                                                 const std::string& target_environment) {
    ASFMLoggerConfiguration deployment_config = config;

    // Apply environment-specific modifications
    if (target_environment == "PROD") {
        deployment_config.enable_health_checks = true;
        deployment_config.health_check_interval_seconds = 30;
        strcpy_s(deployment_config.alert_webhook_url, "https://monitoring.company.com/alerts");
    }

    // Sanitize sensitive information for deployment
    strcpy_s(deployment_config.database_password, "");
    strcpy_s(deployment_config.encryption_key, "");

    return deployment_config;
}

bool ConfigurationToolbox::ValidateForDeployment(const ASFMLoggerConfiguration& config,
                                                const std::string& target_environment) {
    if (!QuickValidateConfiguration(config)) {
        return false;
    }

    if (target_environment == "PROD") {
        return config.enable_performance_monitoring &&
               config.enable_health_checks &&
               strlen(config.alert_webhook_url) > 0;
    }

    return true;
}

std::string ConfigurationToolbox::GenerateDeploymentScript(const ASFMLoggerConfiguration& config,
                                                         const std::string& target_environment) {
    std::ostringstream oss;

    oss << "#!/bin/bash" << std::endl;
    oss << "# ASFMLogger Deployment Script" << std::endl;
    oss << "# Target Environment: " << target_environment << std::endl;
    oss << std::endl;

    oss << "echo \"Deploying ASFMLogger configuration...\"" << std::endl;
    oss << "echo \"Application: " << config.application_name << "\"" << std::endl;
    oss << "echo \"Environment: " << config.environment << "\"" << std::endl;

    return oss.str();
}

std::string ConfigurationToolbox::GenerateRollbackScript(const ASFMLoggerConfiguration& current_config,
                                                       const ASFMLoggerConfiguration& previous_config) {
    std::ostringstream oss;

    oss << "#!/bin/bash" << std::endl;
    oss << "# ASFMLogger Rollback Script" << std::endl;
    oss << std::endl;

    oss << "echo \"Rolling back ASFMLogger configuration...\"" << std::endl;
    oss << "echo \"From: " << current_config.config_name << "\"" << std::endl;
    oss << "echo \"To: " << previous_config.config_name << "\"" << std::endl;

    return oss.str();
}

// =====================================================================================
// CONFIGURATION ANALYSIS
// =====================================================================================

std::vector<std::string> ConfigurationToolbox::AnalyzeConfigurationForPerformance(const ASFMLoggerConfiguration& config) {
    std::vector<std::string> suggestions;

    if (config.min_console_level < LogMessageType::INFO) {
        suggestions.push_back("Consider increasing console log level for better performance");
    }

    if (config.max_file_size_bytes > 100 * 1024 * 1024) {
        suggestions.push_back("Large log files may impact performance");
    }

    if (config.queue_flush_interval_ms < 1000) {
        suggestions.push_back("Very frequent queue flushing may impact performance");
    }

    return suggestions;
}

std::vector<std::string> ConfigurationToolbox::AnalyzeConfigurationForSecurity(const ASFMLoggerConfiguration& config) {
    std::vector<std::string> issues;

    if (!config.enable_encryption && config.database_logging_enabled) {
        issues.push_back("Database logging without encryption may expose sensitive data");
    }

    if (config.log_retention_days > 365) {
        issues.push_back("Long log retention period may violate data protection policies");
    }

    if (!config.enable_audit_trail) {
        issues.push_back("Audit trail disabled - configuration changes cannot be tracked");
    }

    return issues;
}

std::vector<std::string> ConfigurationToolbox::AnalyzeConfigurationForResources(const ASFMLoggerConfiguration& config) {
    std::vector<std::string> analysis;

    DWORD complexity = CalculateConfigurationComplexity(config);

    if (complexity > 80) {
        analysis.push_back("High configuration complexity may increase resource usage");
    }

    if (config.default_queue_size > 10000) {
        analysis.push_back("Large queue size may increase memory usage");
    }

    if (config.performance_monitoring_interval_ms < 5000) {
        analysis.push_back("Frequent performance monitoring may increase CPU usage");
    }

    return analysis;
}

DWORD ConfigurationToolbox::CalculateConfigurationComplexity(const ASFMLoggerConfiguration& config) {
    DWORD complexity = 0;

    // Base complexity
    complexity += 10;

    // Feature complexity
    if (config.enhanced_features_enabled) complexity += 20;
    if (config.database_logging_enabled) complexity += 15;
    if (config.shared_memory_enabled) complexity += 10;
    if (config.smart_queue_enabled) complexity += 15;

    // Security complexity
    if (config.enable_encryption) complexity += 10;
    if (config.enable_audit_trail) complexity += 5;

    // Monitoring complexity
    if (config.enable_performance_monitoring) complexity += 10;
    if (config.enable_health_checks) complexity += 5;

    return (complexity < DWORD(100)) ? complexity : DWORD(100);
}

std::vector<std::string> ConfigurationToolbox::GetConfigurationRecommendations(const ASFMLoggerConfiguration& config,
                                                                             const std::string& use_case) {
    std::vector<std::string> recommendations;

    if (use_case == "HIGH_PERFORMANCE") {
        if (config.min_console_level < LogMessageType::WARN) {
            recommendations.push_back("Increase console log level to WARN for better performance");
        }
        if (config.queue_flush_interval_ms > 10000) {
            recommendations.push_back("Decrease queue flush interval for better performance");
        }
    }
    else if (use_case == "COMPLIANCE") {
        if (!config.enable_audit_trail) {
            recommendations.push_back("Enable audit trail for compliance requirements");
        }
        if (!config.enable_encryption) {
            recommendations.push_back("Enable encryption for sensitive data protection");
        }
    }

    return recommendations;
}

// =====================================================================================
// UTILITY FUNCTIONS
// =====================================================================================

std::string ConfigurationToolbox::ValidationResultToString(const ConfigurationValidationResult& result) {
    std::ostringstream oss;

    oss << "Validation Result:" << std::endl;
    oss << "Valid: " << (result.is_valid ? "Yes" : "No") << std::endl;
    oss << "Errors: " << result.error_count << std::endl;
    oss << "Warnings: " << result.warning_count << std::endl;

    if (strlen(result.primary_error) > 0) {
        oss << "Primary Error: " << result.primary_error << std::endl;
    }

    return oss.str();
}

std::string ConfigurationToolbox::ConfigurationToString(const ASFMLoggerConfiguration& config,
                                                       bool include_sensitive) {
    std::ostringstream oss;

    oss << "Configuration: " << config.config_name << std::endl;
    oss << "Application: " << config.application_name << std::endl;
    oss << "Environment: " << config.environment << std::endl;
    oss << "Console Output: " << (config.console_output_enabled ? "Enabled" : "Disabled") << std::endl;
    oss << "Log Level: " << static_cast<int>(config.min_console_level) << std::endl;

    if (include_sensitive && strlen(config.database_server) > 0) {
        oss << "Database Server: " << config.database_server << std::endl;
    }

    return oss.str();
}

std::string ConfigurationToolbox::TemplateToString(const ConfigurationTemplate& config_template) {
    std::ostringstream oss;

    oss << "Template: " << config_template.template_name << std::endl;
    oss << "Category: " << config_template.template_category << std::endl;
    oss << "Author: " << config_template.author << std::endl;
    oss << "Version: " << config_template.version << std::endl;

    return oss.str();
}

DWORD ConfigurationToolbox::GetCurrentTimestamp() {
    return LoggerInstanceToolbox::GetCurrentTimestamp();
}

uint32_t ConfigurationToolbox::GenerateConfigurationId() {
    static uint32_t next_id = 1;
    return next_id++;
}

// =====================================================================================
// PRIVATE HELPER METHODS
// =====================================================================================

#ifdef ASFMLOGGER_JSON_SUPPORT
nlohmann::json ConfigurationToolbox::ParseJsonConfig(const std::string& json_config) {
    return nlohmann::json::parse(json_config);
}

ASFMLoggerConfiguration ConfigurationToolbox::JsonToConfiguration(const nlohmann::json& json_config) {
    ASFMLoggerConfiguration config = CreateDefaultConfiguration("JSON", "DEV");

    if (json_config.contains("application_name")) {
        strcpy_s(config.application_name, json_config["application_name"].get<std::string>().c_str());
    }

    if (json_config.contains("environment")) {
        strcpy_s(config.environment, json_config["environment"].get<std::string>().c_str());
    }

    if (json_config.contains("console_output_enabled")) {
        config.console_output_enabled = json_config["console_output_enabled"].get<bool>();
    }

    return config;
}

nlohmann::json ConfigurationToolbox::ConfigurationToJson(const ASFMLoggerConfiguration& config) {
    nlohmann::json json_obj;

    json_obj["config_id"] = config.config_id;
    json_obj["config_name"] = config.config_name;
    json_obj["application_name"] = config.application_name;
    json_obj["environment"] = config.environment;
    json_obj["console_output_enabled"] = config.console_output_enabled;

    return json_obj;
}
#endif

std::string ConfigurationToolbox::GetEnvironmentVariable(const std::string& variable_name) {
#ifdef _WIN32
    char* value;
    size_t size;
    errno_t err = _dupenv_s(&value, &size, variable_name.c_str());
    if (err == 0 && value != nullptr) {
        std::string result(value);
        free(value);
        return result;
    }
#endif
    return "";
}

bool ConfigurationToolbox::SetEnvironmentVariable(const std::string& variable_name, const std::string& value) {
#ifdef _WIN32
    return _putenv_s(variable_name.c_str(), value.c_str()) == 0;
#endif
    return false;
}

std::vector<std::string> ConfigurationToolbox::ParseCommandLineArgument(const std::string& arg) {
    std::vector<std::string> parts;
    size_t equal_pos = arg.find('=');

    if (equal_pos != std::string::npos) {
        parts.push_back(arg.substr(0, equal_pos));
        parts.push_back(arg.substr(equal_pos + 1));
    }

    return parts;
}

void ConfigurationToolbox::InitializeDefaultTemplates() {
    // Initialize default templates
}

bool ConfigurationToolbox::IsInitialized() {
    return !application_configs_.empty();
}

std::string ConfigurationToolbox::SanitizeConfigurationValue(const std::string& value, const std::string& field_name) {
    // Basic sanitization - remove potentially harmful characters
    std::string sanitized = value;

    // Remove null bytes
    sanitized.erase(std::remove(sanitized.begin(), sanitized.end(), '\0'), sanitized.end());

    // Truncate if too long
    if (sanitized.length() > 1000) {
        sanitized = sanitized.substr(0, 1000);
    }

    return sanitized;
}