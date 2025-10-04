#ifndef __ASFM_LOGGER_CONFIGURATION_TOOLBOX_HPP__
#define __ASFM_LOGGER_CONFIGURATION_TOOLBOX_HPP__

/**
 * ASFMLogger Configuration Toolbox
 *
 * Static methods for configuration management following toolbox architecture.
 * Pure functions for parsing, validation, and configuration processing.
 */

#include "structs/LogDataStructures.hpp"
#include "structs/ConfigurationData.hpp"
#include <string>
#include <vector>
#include <unordered_map>

// Optional JSON support - only include if available
#ifdef ASFMLOGGER_JSON_SUPPORT
#include <nlohmann/json.hpp>
#endif

// Forward declarations
struct ASFMLoggerConfiguration;
struct ConfigurationValidationResult;
struct ConfigurationTemplate;
struct ConfigurationChange;
struct ConfigurationEnvironmentSettings;

class ConfigurationToolbox {
private:
    // Only static variables allowed in toolbox
    static std::unordered_map<std::string, ASFMLoggerConfiguration> application_configs_;
    static std::unordered_map<std::string, ConfigurationTemplate> configuration_templates_;
    static std::vector<ConfigurationChange> configuration_changes_;

public:
    // =================================================================================
    // CONFIGURATION PARSING
    // =================================================================================

    /**
     * @brief Parse configuration from JSON string
     * @param json_config JSON configuration string
     * @return Parsed configuration or default configuration if parsing fails
     */
    static ASFMLoggerConfiguration ParseConfigurationFromJson(const std::string& json_config);

    /**
     * @brief Parse configuration from JSON file
     * @param config_file Path to JSON configuration file
     * @return Parsed configuration or default configuration if parsing fails
     */
    static ASFMLoggerConfiguration ParseConfigurationFromFile(const std::string& config_file);

    /**
     * @brief Parse configuration from XML string
     * @param xml_config XML configuration string
     * @return Parsed configuration or default configuration if parsing fails
     */
    static ASFMLoggerConfiguration ParseConfigurationFromXml(const std::string& xml_config);

    /**
     * @brief Parse configuration from environment variables
     * @param application_name Application name to parse config for
     * @return Parsed configuration from environment variables
     */
    static ASFMLoggerConfiguration ParseConfigurationFromEnvironment(const std::string& application_name);

    /**
     * @brief Parse configuration from command line arguments
     * @param argc Argument count
     * @param argv Argument values
     * @return Parsed configuration from command line
     */
    static ASFMLoggerConfiguration ParseConfigurationFromCommandLine(int argc, char* argv[]);

    // =================================================================================
    // CONFIGURATION VALIDATION
    // =================================================================================

    /**
     * @brief Validate complete configuration
     * @param config Configuration to validate
     * @return Detailed validation result
     */
    static ConfigurationValidationResult ValidateConfiguration(const ASFMLoggerConfiguration& config);

    /**
     * @brief Validate application settings section
     * @param config Configuration to validate
     * @return true if application settings are valid
     */
    static bool ValidateApplicationSettings(const ASFMLoggerConfiguration& config);

    /**
     * @brief Validate logging settings section
     * @param config Configuration to validate
     * @return true if logging settings are valid
     */
    static bool ValidateLoggingSettings(const ASFMLoggerConfiguration& config);

    /**
     * @brief Validate database settings section
     * @param config Configuration to validate
     * @return true if database settings are valid
     */
    static bool ValidateDatabaseSettings(const ASFMLoggerConfiguration& config);

    /**
     * @brief Validate performance settings section
     * @param config Configuration to validate
     * @return true if performance settings are valid
     */
    static bool ValidatePerformanceSettings(const ASFMLoggerConfiguration& config);

    /**
     * @brief Validate security settings section
     * @param config Configuration to validate
     * @return true if security settings are valid
     */
    static bool ValidateSecuritySettings(const ASFMLoggerConfiguration& config);

    /**
     * @brief Quick validation for basic configuration check
     * @param config Configuration to validate
     * @return true if configuration passes basic validation
     */
    static bool QuickValidateConfiguration(const ASFMLoggerConfiguration& config);

    // =================================================================================
    // CONFIGURATION SERIALIZATION
    // =================================================================================

    /**
     * @brief Convert configuration to JSON string
     * @param config Configuration to convert
     * @param pretty_print Whether to format JSON for readability
     * @return JSON string representation
     */
    static std::string ConfigurationToJson(const ASFMLoggerConfiguration& config, bool pretty_print = false);

    /**
     * @brief Convert configuration to XML string
     * @param config Configuration to convert
     * @param pretty_print Whether to format XML for readability
     * @return XML string representation
     */
    static std::string ConfigurationToXml(const ASFMLoggerConfiguration& config, bool pretty_print = false);

    /**
     * @brief Convert configuration to environment variables format
     * @param config Configuration to convert
     * @return Environment variables as formatted string
     */
    static std::string ConfigurationToEnvironmentVariables(const ASFMLoggerConfiguration& config);

    /**
     * @brief Convert configuration to command line arguments
     * @param config Configuration to convert
     * @return Command line arguments as formatted string
     */
    static std::string ConfigurationToCommandLine(const ASFMLoggerConfiguration& config);

    /**
     * @brief Save configuration to JSON file
     * @param config Configuration to save
     * @param file_path Path to save file
     * @param pretty_print Whether to format JSON for readability
     * @return true if configuration saved successfully
     */
    static bool SaveConfigurationToFile(const ASFMLoggerConfiguration& config,
                                       const std::string& file_path,
                                       bool pretty_print = false);

    // =================================================================================
    // DEFAULT CONFIGURATION CREATION
    // =================================================================================

    /**
     * @brief Create default configuration for an application
     * @param application_name Name of the application
     * @param environment Environment ("DEV", "TEST", "PROD")
     * @return Default configuration
     */
    static ASFMLoggerConfiguration CreateDefaultConfiguration(const std::string& application_name,
                                                            const std::string& environment = "DEV");

    /**
     * @brief Create high-performance configuration
     * @param application_name Name of the application
     * @param environment Environment
     * @return High-performance configuration
     */
    static ASFMLoggerConfiguration CreateHighPerformanceConfiguration(const std::string& application_name,
                                                                     const std::string& environment = "PROD");

    /**
     * @brief Create comprehensive logging configuration
     * @param application_name Name of the application
     * @param environment Environment
     * @return Comprehensive logging configuration
     */
    static ASFMLoggerConfiguration CreateComprehensiveConfiguration(const std::string& application_name,
                                                                   const std::string& environment = "PROD");

    /**
     * @brief Create development configuration
     * @param application_name Name of the application
     * @return Development-optimized configuration
     */
    static ASFMLoggerConfiguration CreateDevelopmentConfiguration(const std::string& application_name);

    /**
     * @brief Create production configuration
     * @param application_name Name of the application
     * @return Production-optimized configuration
     */
    static ASFMLoggerConfiguration CreateProductionConfiguration(const std::string& application_name);

    // =================================================================================
    // CONFIGURATION MERGING AND OVERRIDES
    // =================================================================================

    /**
     * @brief Merge two configurations (base + override)
     * @param base_config Base configuration
     * @param override_config Override configuration
     * @return Merged configuration
     */
    static ASFMLoggerConfiguration MergeConfigurations(const ASFMLoggerConfiguration& base_config,
                                                      const ASFMLoggerConfiguration& override_config);

    /**
     * @brief Apply environment-specific overrides
     * @param base_config Base configuration
     * @param environment_settings Environment-specific settings
     * @return Configuration with environment overrides applied
     */
    static ASFMLoggerConfiguration ApplyEnvironmentOverrides(const ASFMLoggerConfiguration& base_config,
                                                           const ConfigurationEnvironmentSettings& environment_settings);

    /**
     * @brief Apply application-specific overrides
     * @param base_config Base configuration
     * @param application_name Application name for overrides
     * @return Configuration with application overrides applied
     */
    static ASFMLoggerConfiguration ApplyApplicationOverrides(const ASFMLoggerConfiguration& base_config,
                                                           const std::string& application_name);

    /**
     * @brief Extract configuration differences
     * @param config1 First configuration
     * @param config2 Second configuration
     * @return List of differences between configurations
     */
    static std::vector<std::string> ExtractConfigurationDifferences(const ASFMLoggerConfiguration& config1,
                                                                   const ASFMLoggerConfiguration& config2);

    // =================================================================================
    // TEMPLATE MANAGEMENT
    // =================================================================================

    /**
     * @brief Create configuration from template
     * @param template_name Template name to use
     * @param application_name Application name
     * @param customizations Custom configuration overrides
     * @return Configuration based on template
     */
    static ASFMLoggerConfiguration CreateFromTemplate(const std::string& template_name,
                                                     const std::string& application_name,
                                                     const std::unordered_map<std::string, std::string>& customizations = {});

    /**
     * @brief Save configuration as template
     * @param config Configuration to save as template
     * @param template_name Template name
     * @param category Template category
     * @return true if template saved successfully
     */
    static bool SaveAsTemplate(const ASFMLoggerConfiguration& config,
                              const std::string& template_name,
                              const std::string& category);

    /**
     * @brief Get available configuration templates
     * @return Vector of available template names
     */
    static std::vector<std::string> GetAvailableTemplates();

    /**
     * @brief Get templates by category
     * @param category Template category
     * @return Vector of template names in category
     */
    static std::vector<std::string> GetTemplatesByCategory(const std::string& category);

    /**
     * @brief Validate template
     * @param config Configuration to validate as template
     * @return true if configuration can be used as template
     */
    static bool ValidateTemplate(const ASFMLoggerConfiguration& config);

    // =================================================================================
    // CONFIGURATION CHANGE TRACKING
    // =================================================================================

    /**
     * @brief Track configuration change
     * @param old_config Previous configuration
     * @param new_config New configuration
     * @param change_description Description of the change
     * @param changed_by Who made the change
     * @return true if change was tracked successfully
     */
    static bool TrackConfigurationChange(const ASFMLoggerConfiguration& old_config,
                                        const ASFMLoggerConfiguration& new_config,
                                        const std::string& change_description,
                                        const std::string& changed_by);

    /**
     * @brief Get configuration change history
     * @param config_id Configuration ID to get history for
     * @param max_entries Maximum number of entries to return
     * @return Vector of configuration changes
     */
    static std::vector<ConfigurationChange> GetConfigurationChangeHistory(uint32_t config_id,
                                                                         size_t max_entries = 50);

    /**
     * @brief Get configuration changes by user
     * @param changed_by User who made the changes
     * @param max_entries Maximum number of entries to return
     * @return Vector of configuration changes by user
     */
    static std::vector<ConfigurationChange> GetConfigurationChangesByUser(const std::string& changed_by,
                                                                          size_t max_entries = 50);

    /**
     * @brief Get configuration changes in time range
     * @param start_time Start of time range
     * @param end_time End of time range
     * @return Vector of configuration changes in time range
     */
    static std::vector<ConfigurationChange> GetConfigurationChangesInTimeRange(DWORD start_time,
                                                                              DWORD end_time);

    // =================================================================================
    // ENVIRONMENT-SPECIFIC CONFIGURATION
    // =================================================================================

    /**
     * @brief Load environment-specific settings
     * @param environment Environment name
     * @param region Geographic region (optional)
     * @return Environment-specific settings
     */
    static ConfigurationEnvironmentSettings LoadEnvironmentSettings(const std::string& environment,
                                                                   const std::string& region = "");

    /**
     * @brief Save environment-specific settings
     * @param environment_settings Settings to save
     * @return true if settings saved successfully
     */
    static bool SaveEnvironmentSettings(const ConfigurationEnvironmentSettings& environment_settings);

    /**
     * @brief Get configuration for specific environment
     * @param base_config Base configuration
     * @param environment Target environment
     * @param region Geographic region (optional)
     * @return Environment-specific configuration
     */
    static ASFMLoggerConfiguration GetEnvironmentConfiguration(const ASFMLoggerConfiguration& base_config,
                                                              const std::string& environment,
                                                              const std::string& region = "");

    /**
     * @brief Validate environment-specific settings
     * @param settings Settings to validate
     * @return true if settings are valid
     */
    static bool ValidateEnvironmentSettings(const ConfigurationEnvironmentSettings& settings);

    // =================================================================================
    // CONFIGURATION DEPLOYMENT
    // =================================================================================

    /**
     * @brief Prepare configuration for deployment
     * @param config Configuration to prepare
     * @param target_environment Target environment
     * @return Deployment-ready configuration
     */
    static ASFMLoggerConfiguration PrepareForDeployment(const ASFMLoggerConfiguration& config,
                                                       const std::string& target_environment);

    /**
     * @brief Validate configuration for deployment
     * @param config Configuration to validate
     * @param target_environment Target environment
     * @return true if configuration is valid for deployment
     */
    static bool ValidateForDeployment(const ASFMLoggerConfiguration& config,
                                      const std::string& target_environment);

    /**
     * @brief Generate deployment script
     * @param config Configuration to deploy
     * @param target_environment Target environment
     * @return Deployment script as formatted string
     */
    static std::string GenerateDeploymentScript(const ASFMLoggerConfiguration& config,
                                               const std::string& target_environment);

    /**
     * @brief Generate rollback script
     * @param current_config Current configuration
     * @param previous_config Previous configuration
     * @return Rollback script as formatted string
     */
    static std::string GenerateRollbackScript(const ASFMLoggerConfiguration& current_config,
                                             const ASFMLoggerConfiguration& previous_config);

    // =================================================================================
    // CONFIGURATION ANALYSIS
    // =================================================================================

    /**
     * @brief Analyze configuration for performance optimization
     * @param config Configuration to analyze
     * @return Performance optimization suggestions
     */
    static std::vector<std::string> AnalyzeConfigurationForPerformance(const ASFMLoggerConfiguration& config);

    /**
     * @brief Analyze configuration for security compliance
     * @param config Configuration to analyze
     * @return Security compliance issues
     */
    static std::vector<std::string> AnalyzeConfigurationForSecurity(const ASFMLoggerConfiguration& config);

    /**
     * @brief Analyze configuration for resource usage
     * @param config Configuration to analyze
     * @return Resource usage analysis
     */
    static std::vector<std::string> AnalyzeConfigurationForResources(const ASFMLoggerConfiguration& config);

    /**
     * @brief Calculate configuration complexity score
     * @param config Configuration to analyze
     * @return Complexity score (0-100, higher = more complex)
     */
    static DWORD CalculateConfigurationComplexity(const ASFMLoggerConfiguration& config);

    /**
     * @brief Get configuration recommendations
     * @param config Current configuration
     * @param use_case Use case ("HIGH_PERFORMANCE", "COMPLIANCE", "DEBUG", "BALANCED")
     * @return Vector of configuration recommendations
     */
    static std::vector<std::string> GetConfigurationRecommendations(const ASFMLoggerConfiguration& config,
                                                                   const std::string& use_case);

    // =================================================================================
    // UTILITY FUNCTIONS
    // =================================================================================

    /**
     * @brief Convert validation result to string
     * @param result Validation result to format
     * @return Human-readable validation description
     */
    static std::string ValidationResultToString(const ConfigurationValidationResult& result);

    /**
     * @brief Convert configuration to string
     * @param config Configuration to format
     * @param include_sensitive Whether to include sensitive information
     * @return Human-readable configuration description
     */
    static std::string ConfigurationToString(const ASFMLoggerConfiguration& config,
                                            bool include_sensitive = false);

    /**
     * @brief Convert template to string
     * @param template Template to format
     * @return Human-readable template description
     */
    static std::string TemplateToString(const ConfigurationTemplate& config_template);

    /**
     * @brief Get current timestamp for configuration operations
     * @return Current timestamp
     */
    static DWORD GetCurrentTimestamp();

    /**
     * @brief Generate unique configuration ID
     * @return Unique configuration identifier
     */
    static uint32_t GenerateConfigurationId();

private:
    // Private helper methods
    static std::string GetEnvironmentVariable(const std::string& variable_name);
    static bool SetEnvironmentVariable(const std::string& variable_name, const std::string& value);
    static std::vector<std::string> ParseCommandLineArgument(const std::string& arg);
    static void InitializeDefaultTemplates();
    static bool IsInitialized();
    static std::string SanitizeConfigurationValue(const std::string& value, const std::string& field_name);

// Optional JSON support methods
#ifdef ASFMLOGGER_JSON_SUPPORT
    static nlohmann::json ParseJsonConfig(const std::string& json_config);
    static ASFMLoggerConfiguration JsonToConfiguration(const nlohmann::json& json_config);
    static nlohmann::json ConfigurationToJson(const ASFMLoggerConfiguration& config);
#endif
};

#endif // __ASFM_LOGGER_CONFIGURATION_TOOLBOX_HPP__