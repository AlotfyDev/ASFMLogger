#ifndef __ASFM_LOGGER_IMPORTANCE_MAPPER_HPP__
#define __ASFM_LOGGER_IMPORTANCE_MAPPER_HPP__

/**
 * ASFMLogger Stateful Importance Mapper
 *
 * Stateful wrapper class for importance configuration management using toolbox internally.
 * Provides object-oriented interface for importance mapping and resolution.
 */

#include "structs/LogDataStructures.hpp"
#include "structs/ImportanceConfiguration.hpp"
#include "toolbox/ImportanceToolbox.hpp"
#include <string>
#include <vector>
#include <memory>
#include <mutex>

class ImportanceMapper {
private:
    std::string application_name_;
    ApplicationImportanceConfig config_;
    mutable std::mutex mapper_mutex_;

public:
    // =================================================================================
    // CONSTRUCTORS AND DESTRUCTOR
    // =================================================================================

    /**
     * @brief Default constructor
     */
    ImportanceMapper();

    /**
     * @brief Constructor with application name
     * @param application_name Name of the application this mapper serves
     */
    explicit ImportanceMapper(const std::string& application_name);

    /**
     * @brief Constructor with application name and configuration
     * @param application_name Name of the application
     * @param config Initial configuration
     */
    ImportanceMapper(const std::string& application_name, const ApplicationImportanceConfig& config);

    /**
     * @brief Copy constructor
     * @param other Mapper to copy from
     */
    ImportanceMapper(const ImportanceMapper& other);

    /**
     * @brief Move constructor
     * @param other Mapper to move from
     */
    ImportanceMapper(ImportanceMapper&& other) noexcept;

    /**
     * @brief Assignment operator
     * @param other Mapper to assign from
     * @return Reference to this mapper
     */
    ImportanceMapper& operator=(const ImportanceMapper& other);

    /**
     * @brief Move assignment operator
     * @param other Mapper to move from
     * @return Reference to this mapper
     */
    ImportanceMapper& operator=(ImportanceMapper&& other) noexcept;

    /**
     * @brief Destructor
     */
    ~ImportanceMapper() = default;

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
        std::lock_guard<std::mutex> lock(mapper_mutex_);
        return application_name_;
    }

    /**
     * @brief Check if mapper is configured for an application
     * @return true if application name is set
     */
    bool isConfigured() const {
        std::lock_guard<std::mutex> lock(mapper_mutex_);
        return !application_name_.empty();
    }

    // =================================================================================
    // TYPE-BASED IMPORTANCE MAPPING
    // =================================================================================

    /**
     * @brief Set importance for a message type
     * @param type Message type
     * @param importance Importance level
     * @param reason Reason for mapping (optional)
     * @return true if mapping was set successfully
     */
    bool setTypeImportance(LogMessageType type, MessageImportance importance,
                          const std::string& reason = "");

    /**
     * @brief Get importance for a message type
     * @param type Message type
     * @return Importance level
     */
    MessageImportance getTypeImportance(LogMessageType type) const;

    /**
     * @brief Reset type importance to default
     * @param type Message type
     * @return true if reset successful
     */
    bool resetTypeImportance(LogMessageType type);

    /**
     * @brief Get all type mappings for this application
     * @return Vector of type-importance pairs
     */
    std::vector<std::pair<LogMessageType, MessageImportance>> getAllTypeMappings() const;

    // =================================================================================
    // COMPONENT IMPORTANCE OVERRIDES
    // =================================================================================

    /**
     * @brief Add component importance override
     * @param component_pattern Component name pattern (supports wildcards)
     * @param importance Importance level for this component
     * @param use_regex Whether pattern should be treated as regex
     * @param reason Reason for override
     * @return Override ID for later reference
     */
    uint32_t addComponentOverride(const std::string& component_pattern,
                                 MessageImportance importance,
                                 bool use_regex = false,
                                 const std::string& reason = "");

    /**
     * @brief Remove component importance override
     * @param override_id Override ID to remove
     * @return true if override was found and removed
     */
    bool removeComponentOverride(uint32_t override_id);

    /**
     * @brief Update component importance override
     * @param override_id Override ID to update
     * @param importance New importance level
     * @param reason New reason (optional)
     * @return true if override was found and updated
     */
    bool updateComponentOverride(uint32_t override_id,
                                MessageImportance importance,
                                const std::string& reason = "");

    /**
     * @brief Get all component overrides for this application
     * @return Vector of component overrides
     */
    std::vector<ComponentImportanceOverride> getComponentOverrides() const;

    /**
     * @brief Find component override for a component name
     * @param component_name Component name to match
     * @return Pointer to override or nullptr if not found
     */
    const ComponentImportanceOverride* findComponentOverride(const std::string& component_name) const;

    // =================================================================================
    // FUNCTION IMPORTANCE OVERRIDES
    // =================================================================================

    /**
     * @brief Add function importance override
     * @param function_pattern Function name pattern (supports wildcards)
     * @param importance Importance level for this function
     * @param use_regex Whether pattern should be treated as regex
     * @param reason Reason for override
     * @return Override ID for later reference
     */
    uint32_t addFunctionOverride(const std::string& function_pattern,
                                MessageImportance importance,
                                bool use_regex = false,
                                const std::string& reason = "");

    /**
     * @brief Remove function importance override
     * @param override_id Override ID to remove
     * @return true if override was found and removed
     */
    bool removeFunctionOverride(uint32_t override_id);

    /**
     * @brief Update function importance override
     * @param override_id Override ID to update
     * @param importance New importance level
     * @param reason New reason (optional)
     * @return true if override was found and updated
     */
    bool updateFunctionOverride(uint32_t override_id,
                               MessageImportance importance,
                               const std::string& reason = "");

    /**
     * @brief Get all function overrides for this application
     * @return Vector of function overrides
     */
    std::vector<FunctionImportanceOverride> getFunctionOverrides() const;

    /**
     * @brief Find function override for a function name
     * @param function_name Function name to match
     * @return Pointer to override or nullptr if not found
     */
    const FunctionImportanceOverride* findFunctionOverride(const std::string& function_name) const;

    // =================================================================================
    // IMPORTANCE RESOLUTION
    // =================================================================================

    /**
     * @brief Resolve message importance using hierarchy: Function > Component > Type > Default
     * @param message Message data to analyze
     * @param context Additional context for resolution
     * @return Resolution result with final importance and reasoning
     */
    ImportanceResolutionResult resolveMessageImportance(
        const LogMessageData& message,
        const ImportanceResolutionContext& context) const;

    /**
     * @brief Resolve importance for message type only (no overrides)
     * @param type Message type
     * @return Importance level
     */
    MessageImportance resolveTypeImportance(LogMessageType type) const {
        return ImportanceToolbox::ResolveTypeImportance(type);
    }

    /**
     * @brief Resolve importance with component override consideration
     * @param type Message type
     * @param component Component name
     * @return Importance level
     */
    MessageImportance resolveComponentImportance(LogMessageType type,
                                                const std::string& component) const {
        return ImportanceToolbox::ResolveComponentImportance(type, component);
    }

    /**
     * @brief Resolve importance with function override consideration
     * @param type Message type
     * @param component Component name
     * @param function Function name
     * @return Importance level
     */
    MessageImportance resolveFunctionImportance(LogMessageType type,
                                               const std::string& component,
                                               const std::string& function) const {
        return ImportanceToolbox::ResolveFunctionImportance(type, component, function);
    }

    /**
     * @brief Resolve importance with full context consideration
     * @param type Message type
     * @param component Component name
     * @param function Function name
     * @param system_load Current system load (0-100)
     * @param error_rate Current error rate
     * @return Importance level
     */
    MessageImportance resolveContextualImportance(LogMessageType type,
                                                 const std::string& component,
                                                 const std::string& function,
                                                 DWORD system_load = 0,
                                                 DWORD error_rate = 0) const {
        return ImportanceToolbox::ResolveContextualImportance(type, component, function,
                                                             application_name_, system_load, error_rate);
    }

    // =================================================================================
    // PERSISTENCE DECISIONS
    // =================================================================================

    /**
     * @brief Determine if message should be persisted to database
     * @param message Message to evaluate
     * @param context Resolution context
     * @param min_persistence_importance Minimum importance for persistence
     * @return true if message should be persisted
     */
    bool shouldPersistMessage(const LogMessageData& message,
                             const ImportanceResolutionContext& context,
                             MessageImportance min_persistence_importance) const {
        return ImportanceToolbox::ShouldPersistMessage(message, context, min_persistence_importance);
    }

    /**
     * @brief Determine if message should be persisted based on component
     * @param component Component name
     * @param message_type Message type
     * @param system_load Current system load
     * @return true if message should be persisted
     */
    bool shouldPersistByComponent(const std::string& component,
                                 LogMessageType message_type,
                                 DWORD system_load = 0) const {
        return ImportanceToolbox::ShouldPersistByComponent(component, message_type, system_load);
    }

    /**
     * @brief Determine if message should be persisted based on system conditions
     * @param message_type Message type
     * @param system_load Current system load (0-100)
     * @param error_rate Current error rate
     * @param is_emergency_mode Whether system is in emergency mode
     * @return true if message should be persisted
     */
    bool shouldPersistBySystemConditions(LogMessageType message_type,
                                        DWORD system_load,
                                        DWORD error_rate,
                                        bool is_emergency_mode) const {
        return ImportanceToolbox::ShouldPersistBySystemConditions(message_type, system_load,
                                                                 error_rate, is_emergency_mode);
    }

    // =================================================================================
    // CONFIGURATION MANAGEMENT
    // =================================================================================

    /**
     * @brief Load configuration from file
     * @param config_file Path to configuration file
     * @return true if configuration loaded successfully
     */
    bool loadFromFile(const std::string& config_file);

    /**
     * @brief Save configuration to file
     * @param config_file Path to save configuration file
     * @return true if configuration saved successfully
     */
    bool saveToFile(const std::string& config_file) const;

    /**
     * @brief Reset configuration to defaults
     * @return true if reset successful
     */
    bool resetToDefaults();

    /**
     * @brief Validate current configuration
     * @return true if configuration is valid
     */
    bool validateConfiguration() const {
        std::lock_guard<std::mutex> lock(mapper_mutex_);
        return ImportanceToolbox::ValidateConfiguration(config_);
    }

    /**
     * @brief Get raw configuration data
     * @return Const reference to configuration
     */
    const ApplicationImportanceConfig& getConfiguration() const {
        std::lock_guard<std::mutex> lock(mapper_mutex_);
        return config_;
    }

    /**
     * @brief Set configuration data
     * @param config New configuration data
     * @return true if configuration is valid and was set
     */
    bool setConfiguration(const ApplicationImportanceConfig& config);

    // =================================================================================
    // STATISTICS AND ANALYSIS
    // =================================================================================

    /**
     * @brief Get importance mapping statistics
     * @return String containing mapping statistics
     */
    std::string getMappingStatistics() const {
        return ImportanceToolbox::GetMappingStatistics();
    }

    /**
     * @brief Get override usage statistics
     * @return String containing override statistics
     */
    std::string getOverrideStatistics() const {
        return ImportanceToolbox::GetOverrideStatistics();
    }

    /**
     * @brief Analyze importance distribution in message batch
     * @param messages Vector of messages to analyze
     * @return Analysis results as formatted string
     */
    std::string analyzeImportanceDistribution(const std::vector<LogMessageData>& messages) const {
        return ImportanceToolbox::AnalyzeImportanceDistribution(messages);
    }

    /**
     * @brief Get most common component overrides
     * @param max_results Maximum number of results to return
     * @return Vector of component-override pairs
     */
    std::vector<std::pair<std::string, MessageImportance>> getMostUsedComponentOverrides(
        size_t max_results = 10) const {
        return ImportanceToolbox::GetMostUsedComponentOverrides(max_results);
    }

    /**
     * @brief Get most common function overrides
     * @param max_results Maximum number of results to return
     * @return Vector of function-override pairs
     */
    std::vector<std::pair<std::string, MessageImportance>> getMostUsedFunctionOverrides(
        size_t max_results = 10) const {
        return ImportanceToolbox::GetMostUsedFunctionOverrides(max_results);
    }

    // =================================================================================
    // BATCH OPERATIONS
    // =================================================================================

    /**
     * @brief Resolve importance for multiple messages
     * @param messages Vector of messages to analyze
     * @param context Resolution context
     * @return Vector of resolution results
     */
    std::vector<ImportanceResolutionResult> resolveBatchImportance(
        const std::vector<LogMessageData>& messages,
        const ImportanceResolutionContext& context) const {
        return ImportanceToolbox::ResolveBatchImportance(messages, context);
    }

    /**
     * @brief Filter messages by minimum importance
     * @param messages Vector of messages to filter
     * @param min_importance Minimum importance level
     * @return Vector of messages meeting importance criteria
     */
    std::vector<LogMessageData> filterByImportance(
        const std::vector<LogMessageData>& messages,
        MessageImportance min_importance) const {
        return ImportanceToolbox::FilterByImportance(messages, min_importance);
    }

    /**
     * @brief Count messages by importance level
     * @param messages Vector of messages to analyze
     * @return Vector of pairs (importance, count)
     */
    std::vector<std::pair<MessageImportance, size_t>> countByImportance(
        const std::vector<LogMessageData>& messages) const {
        return ImportanceToolbox::CountByImportance(messages);
    }

    // =================================================================================
    // UTILITY METHODS
    // =================================================================================

    /**
     * @brief Clear all overrides for this application
     * @return Number of overrides removed
     */
    size_t clearAllOverrides();

    /**
     * @brief Clear component overrides only
     * @return Number of overrides removed
     */
    size_t clearComponentOverrides();

    /**
     * @brief Clear function overrides only
     * @return Number of overrides removed
     */
    size_t clearFunctionOverrides();

    /**
     * @brief Get total number of overrides
     * @return Total override count
     */
    size_t getOverrideCount() const;

    /**
     * @brief Check if application has any overrides
     * @return true if application has overrides configured
     */
    bool hasOverrides() const;

    /**
     * @brief Swap contents with another mapper
     * @param other Mapper to swap with
     */
    void swap(ImportanceMapper& other) noexcept;

    /**
     * @brief Clone this mapper configuration
     * @return New ImportanceMapper with same configuration
     */
    ImportanceMapper clone() const;
};

// =====================================================================================
// NON-MEMBER FUNCTIONS
// =====================================================================================

/**
 * @brief Swap two ImportanceMapper objects
 * @param a First mapper
 * @param b Second mapper
 */
void swap(ImportanceMapper& a, ImportanceMapper& b) noexcept;

/**
 * @brief Output stream operator for ImportanceMapper
 * @param os Output stream
 * @param mapper Mapper to output
 * @return Reference to output stream
 */
std::ostream& operator<<(std::ostream& os, const ImportanceMapper& mapper);

/**
 * @brief Create importance mapper for current application
 * @param application_name Name of the application
 * @return New ImportanceMapper for current application
 */
ImportanceMapper CreateApplicationImportanceMapper(const std::string& application_name);

#endif // __ASFM_LOGGER_IMPORTANCE_MAPPER_HPP__