#ifndef __ASFM_LOGGER_CONFIGURATION_MANAGER_HPP__
#define __ASFM_LOGGER_CONFIGURATION_MANAGER_HPP__

/**
 * ASFMLogger Configuration Manager
 *
 * Stateful manager class for configuration lifecycle management.
 * Uses ConfigurationToolbox internally for all configuration operations.
 */

#include "structs/ConfigurationData.hpp"
#include "toolbox/ConfigurationToolbox.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <atomic>

class ConfigurationManager {
private:
    std::string application_name_;
    ASFMLoggerConfiguration current_config_;
    std::vector<ASFMLoggerConfiguration> config_history_;
    mutable std::mutex config_mutex_;

    // Configuration state
    bool is_loaded_;
    std::string config_source_;
    DWORD last_reload_time_;

    // Change tracking
    std::vector<ConfigurationChange> change_history_;
    std::atomic<uint32_t> change_count_;

    // Environment settings
    ConfigurationEnvironmentSettings environment_settings_;
    bool environment_override_active_;

    // Template management
    std::unordered_map<std::string, ConfigurationTemplate> available_templates_;

public:
    // =================================================================================
    // CONSTRUCTORS AND DESTRUCTOR
    // =================================================================================

    /**
     * @brief Default constructor
     */
    ConfigurationManager();

    /**
     * @brief Constructor with application name
     * @param application_name Name of the application
     */
    explicit ConfigurationManager(const std::string& application_name);

    /**
     * @brief Constructor with application name and initial configuration
     * @param application_name Name of the application
     * @param initial_config Initial configuration
     */
    ConfigurationManager(const std::string& application_name, const ASFMLoggerConfiguration& initial_config);

    /**
     * @brief Copy constructor
     * @param other Manager to copy from
     */
    ConfigurationManager(const ConfigurationManager& other);

    /**
     * @brief Move constructor
     * @param other Manager to move from
     */
    ConfigurationManager(ConfigurationManager&& other) noexcept;

    /**
     * @brief Assignment operator
     * @param other Manager to assign from
     * @return Reference to this manager
     */
    ConfigurationManager& operator=(const ConfigurationManager& other);

    /**
     * @brief Move assignment operator
     * @param other Manager to move from
     * @return Reference to this manager
     */
    ConfigurationManager& operator=(ConfigurationManager&& other) noexcept;

    /**
     * @brief Destructor
     */
    ~ConfigurationManager();

    // =================================================================================
    // APPLICATION MANAGEMENT
    // =================================================================================

    /**
     * @brief Set application name
     * @param application_name Name of the application
     */
    void setApplicationName(const std::string& application_name);

    /**
     * @brief Get application name
     * @return Application name
     */
    std::string getApplicationName() const {
        std::lock_guard<std::mutex> lock(config_mutex_);
        return application_name_;
    }

    /**
     * @brief Check if manager is configured for an application
     * @return true if application name is set
     */
    bool isConfigured() const {
        std::lock_guard<std::mutex> lock(config_mutex_);
        return !application_name_.empty() && is_loaded_;
    }

    // =================================================================================
    // CONFIGURATION LOADING
    // =================================================================================

    /**
     * @brief Load configuration from JSON file
     * @param config_file Path to JSON configuration file
     * @return true if configuration loaded successfully
     */
    bool loadFromFile(const std::string& config_file);

    /**
     * @brief Load configuration from JSON string
     * @param json_config JSON configuration string
     * @return true if configuration loaded successfully
     */
    bool loadFromJson(const std::string& json_config);

    /**
     * @brief Load configuration from environment variables
     * @return true if configuration loaded successfully
     */
    bool loadFromEnvironment();

    /**
     * @brief Load configuration from command line arguments
     * @param argc Argument count
     * @param argv Argument values
     * @return true if configuration loaded successfully
     */
    bool loadFromCommandLine(int argc, char* argv[]);

    /**
     * @brief Load configuration from template
     * @param template_name Template name to load from
     * @param customizations Custom configuration overrides
     * @return true if configuration loaded successfully
     */
    bool loadFromTemplate(const std::string& template_name,
                         const std::unordered_map<std::string, std::string>& customizations = {});

    /**
     * @brief Load default configuration for environment
     * @param environment Environment ("DEV", "TEST", "PROD")
     * @return true if default configuration loaded successfully
     */
    bool loadDefaultConfiguration(const std::string& environment = "DEV");

    // =================================================================================
    // CONFIGURATION SAVING
    // =================================================================================

    /**
     * @brief Save current configuration to JSON file
     * @param config_file Path to save configuration file
     * @param pretty_print Whether to format JSON for readability
     * @return true if configuration saved successfully
     */
    bool saveToFile(const std::string& config_file, bool pretty_print = false) const;

    /**
     * @brief Save current configuration as template
     * @param template_name Template name
     * @param category Template category
     * @return true if configuration saved as template successfully
     */
    bool saveAsTemplate(const std::string& template_name, const std::string& category);

    /**
     * @brief Export configuration to JSON string
     * @param pretty_print Whether to format JSON for readability
     * @return JSON string representation
     */
    std::string exportToJson(bool pretty_print = false) const;

    /**
     * @brief Export configuration to environment variables format
     * @return Environment variables as formatted string
     */
    std::string exportToEnvironmentVariables() const;

    // =================================================================================
    // CONFIGURATION MANAGEMENT
    // =================================================================================

    /**
     * @brief Get current configuration
     * @return Current configuration
     */
    ASFMLoggerConfiguration getConfiguration() const {
        std::lock_guard<std::mutex> lock(config_mutex_);
        return current_config_;
    }

    /**
     * @brief Set configuration
     * @param config New configuration
     * @param track_change Whether to track this change
     * @return true if configuration is valid and was set
     */
    bool setConfiguration(const ASFMLoggerConfiguration& config, bool track_change = true);

    /**
     * @brief Update configuration section
     * @param section_name Section to update ("LOGGING", "DATABASE", "PERFORMANCE")
     * @param updates Map of setting names to new values
     * @return true if configuration updated successfully
     */
    bool updateConfigurationSection(const std::string& section_name,
                                   const std::unordered_map<std::string, std::string>& updates);

    /**
     * @brief Reset configuration to defaults
     * @param environment Environment for defaults ("DEV", "TEST", "PROD")
     * @return true if reset successful
     */
    bool resetToDefaults(const std::string& environment = "DEV");

    /**
     * @brief Validate current configuration
     * @return Validation result with detailed error information
     */
    ConfigurationValidationResult validateConfiguration() const;

    // =================================================================================
    // CONFIGURATION HISTORY
    // =================================================================================

    /**
     * @brief Get configuration history
     * @param max_entries Maximum number of entries to return
     * @return Vector of historical configurations
     */
    std::vector<ASFMLoggerConfiguration> getConfigurationHistory(size_t max_entries = 10) const;

    /**
     * @brief Get configuration change history
     * @param max_entries Maximum number of entries to return
     * @return Vector of configuration changes
     */
    std::vector<ConfigurationChange> getChangeHistory(size_t max_entries = 50) const;

    /**
     * @brief Get configuration changes by user
     * @param changed_by User who made the changes
     * @param max_entries Maximum number of entries to return
     * @return Vector of configuration changes by user
     */
    std::vector<ConfigurationChange> getChangesByUser(const std::string& changed_by,
                                                     size_t max_entries = 50) const;

    /**
     * @brief Rollback to previous configuration
     * @param steps Number of steps to rollback (1 = previous, 2 = one before that, etc.)
     * @return true if rollback successful
     */
    bool rollbackConfiguration(size_t steps = 1);

    /**
     * @brief Clear configuration history
     * @return Number of history entries removed
     */
    size_t clearConfigurationHistory();

    // =================================================================================
    // TEMPLATE MANAGEMENT
    // =================================================================================

    /**
     * @brief Get available configuration templates
     * @return Vector of available template names
     */
    std::vector<std::string> getAvailableTemplates() const;

    /**
     * @brief Get templates by category
     * @param category Template category
     * @return Vector of template names in category
     */
    std::vector<std::string> getTemplatesByCategory(const std::string& category) const;

    /**
     * @brief Create configuration from template
     * @param template_name Template name to use
     * @param customizations Custom configuration overrides
     * @return true if configuration created successfully
     */
    bool createFromTemplate(const std::string& template_name,
                           const std::unordered_map<std::string, std::string>& customizations = {});

    /**
     * @brief Save current configuration as template
     * @param template_name Template name
     * @param category Template category
     * @param description Template description
     * @return true if template saved successfully
     */
    bool saveCurrentAsTemplate(const std::string& template_name,
                              const std::string& category,
                              const std::string& description = "");

    /**
     * @brief Delete configuration template
     * @param template_name Template name to delete
     * @return true if template was found and deleted
     */
    bool deleteTemplate(const std::string& template_name);

    // =================================================================================
    // ENVIRONMENT MANAGEMENT
    // =================================================================================

    /**
     * @brief Load environment-specific settings
     * @param environment Environment name
     * @param region Geographic region (optional)
     * @return true if environment settings loaded successfully
     */
    bool loadEnvironmentSettings(const std::string& environment, const std::string& region = "");

    /**
     * @brief Save environment-specific settings
     * @param environment_settings Settings to save
     * @return true if settings saved successfully
     */
    bool saveEnvironmentSettings(const ConfigurationEnvironmentSettings& environment_settings);

    /**
     * @brief Apply environment overrides to current configuration
     * @return true if overrides applied successfully
     */
    bool applyEnvironmentOverrides();

    /**
     * @brief Get current environment settings
     * @return Current environment settings
     */
    ConfigurationEnvironmentSettings getEnvironmentSettings() const {
        std::lock_guard<std::mutex> lock(config_mutex_);
        return environment_settings_;
    }

    /**
     * @brief Set environment override state
     * @param enabled Whether environment overrides are active
     */
    void setEnvironmentOverrideEnabled(bool enabled);

    /**
     * @brief Check if environment override is active
     * @return true if environment override is active
     */
    bool isEnvironmentOverrideActive() const {
        std::lock_guard<std::mutex> lock(config_mutex_);
        return environment_override_active_;
    }

    // =================================================================================
    // CONFIGURATION ANALYSIS
    // =================================================================================

    /**
     * @brief Analyze current configuration for performance
     * @return Performance optimization suggestions
     */
    std::vector<std::string> analyzeForPerformance();

    /**
     * @brief Analyze current configuration for security
     * @return Security compliance issues
     */
    std::vector<std::string> analyzeForSecurity();

    /**
     * @brief Analyze current configuration for resource usage
     * @return Resource usage analysis
     */
    std::vector<std::string> analyzeForResources();

    /**
     * @brief Get configuration recommendations
     * @param use_case Use case ("HIGH_PERFORMANCE", "COMPLIANCE", "DEBUG", "BALANCED")
     * @return Vector of configuration recommendations
     */
    std::vector<std::string> getRecommendations(const std::string& use_case);

    /**
     * @brief Calculate configuration complexity score
     * @return Complexity score (0-100, higher = more complex)
     */
    DWORD calculateComplexityScore();

    // =================================================================================
    // CONFIGURATION DEPLOYMENT
    // =================================================================================

    /**
     * @brief Prepare configuration for deployment
     * @param target_environment Target environment
     * @return Deployment-ready configuration
     */
    ASFMLoggerConfiguration prepareForDeployment(const std::string& target_environment);

    /**
     * @brief Validate configuration for deployment
     * @param target_environment Target environment
     * @return true if configuration is valid for deployment
     */
    bool validateForDeployment(const std::string& target_environment);

    /**
     * @brief Generate deployment script
     * @param target_environment Target environment
     * @return Deployment script as formatted string
     */
    std::string generateDeploymentScript(const std::string& target_environment);

    /**
     * @brief Generate rollback script
     * @return Rollback script as formatted string
     */
    std::string generateRollbackScript();

    // =================================================================================
    // HOT RELOADING
    // =================================================================================

    /**
     * @brief Enable hot reloading of configuration
     * @param config_file Path to configuration file to watch
     * @param reload_interval_seconds How often to check for changes
     * @return true if hot reloading enabled successfully
     */
    bool enableHotReloading(const std::string& config_file, DWORD reload_interval_seconds = 30);

    /**
     * @brief Disable hot reloading
     * @return true if hot reloading disabled successfully
     */
    bool disableHotReloading();

    /**
     * @brief Check if hot reloading is enabled
     * @return true if hot reloading is enabled
     */
    bool isHotReloadingEnabled() const;

    /**
     * @brief Manually trigger configuration reload
     * @return true if reload successful
     */
    bool reloadConfiguration();

    /**
     * @brief Get time since last reload
     * @return Seconds since last configuration reload
     */
    DWORD getTimeSinceLastReload() const;

    // =================================================================================
    // STATISTICS AND MONITORING
    // =================================================================================

    /**
     * @brief Get configuration manager statistics
     * @return String containing manager statistics
     */
    std::string getManagerStatistics() const;

    /**
     * @brief Get configuration change count
     * @return Total number of configuration changes
     */
    uint32_t getChangeCount() const {
        return change_count_;
    }

    /**
     * @brief Get configuration history size
     * @return Number of historical configurations stored
     */
    size_t getConfigurationHistorySize() const {
        std::lock_guard<std::mutex> lock(config_mutex_);
        return config_history_.size();
    }

    /**
     * @brief Get available template count
     * @return Number of available configuration templates
     */
    size_t getTemplateCount() const {
        std::lock_guard<std::mutex> lock(config_mutex_);
        return available_templates_.size();
    }

    /**
     * @brief Get configuration source information
     * @return Information about where configuration was loaded from
     */
    std::string getConfigurationSource() const {
        std::lock_guard<std::mutex> lock(config_mutex_);
        return config_source_;
    }

    // =================================================================================
    // UTILITY METHODS
    // =================================================================================

    /**
     * @brief Clone this configuration manager
     * @return New ConfigurationManager with same configuration
     */
    ConfigurationManager clone() const;

    /**
     * @brief Swap contents with another manager
     * @param other Manager to swap with
     */
    void swap(ConfigurationManager& other) noexcept;

    /**
     * @brief Clear all configurations and history
     */
    void clear();

    /**
     * @brief Check if manager has unsaved changes
     * @return true if there are unsaved changes
     */
    bool hasUnsavedChanges() const;

    /**
     * @brief Get manager status as formatted string
     * @return Status information
     */
    std::string getStatus() const;

    /**
     * @brief Export complete configuration state
     * @return Complete configuration state as formatted string
     */
    std::string exportCompleteState() const;

private:
    // Private helper methods
    void initializeWithDefaults();
    void trackConfigurationChange(const ASFMLoggerConfiguration& old_config,
                                  const ASFMLoggerConfiguration& new_config,
                                  const std::string& change_description);
    bool applyConfiguration(const ASFMLoggerConfiguration& config);
    void updateConfigurationHistory(const ASFMLoggerConfiguration& config);
    void loadAvailableTemplates();
    bool detectConfigurationChanges(const std::string& config_file);
    void performHotReloadCheck();
    DWORD getCurrentTimestamp() const;
};

// =====================================================================================
// GLOBAL FUNCTIONS
// =====================================================================================

/**
 * @brief Get global configuration manager for an application
 * @param application_name Name of the application
 * @return Reference to global ConfigurationManager
 */
ConfigurationManager& GetGlobalConfigurationManager(const std::string& application_name);

/**
 * @brief Create configuration manager for current application
 * @param application_name Name of the application
 * @return New ConfigurationManager for current application
 */
ConfigurationManager CreateApplicationConfigurationManager(const std::string& application_name);

/**
 * @brief Load configuration for application from file
 * @param application_name Name of the application
 * @param config_file Path to configuration file
 * @return true if configuration loaded successfully
 */
bool LoadApplicationConfiguration(const std::string& application_name, const std::string& config_file);

#endif // __ASFM_LOGGER_CONFIGURATION_MANAGER_HPP__