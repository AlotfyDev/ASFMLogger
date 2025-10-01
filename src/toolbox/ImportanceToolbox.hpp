#ifndef __ASFM_LOGGER_IMPORTANCE_TOOLBOX_HPP__
#define __ASFM_LOGGER_IMPORTANCE_TOOLBOX_HPP__

/**
 * ASFMLogger Importance Framework Toolbox
 *
 * Static methods for importance mapping and resolution following toolbox architecture.
 * Pure functions for smart message classification and contextual importance decisions.
 */

#include "structs/LogDataStructures.hpp"
#include "structs/ImportanceConfiguration.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>

// Forward declarations
struct LogMessageData;
struct ImportanceMapping;
struct ComponentImportanceOverride;
struct FunctionImportanceOverride;
struct ApplicationImportanceConfig;
struct ImportanceResolutionContext;
struct ImportanceResolutionResult;

class ImportanceToolbox {
private:
    // Only static variables allowed in toolbox
    static std::unordered_map<LogMessageType, MessageImportance> default_type_mapping_;
    static std::unordered_map<std::string, ComponentImportanceOverride> component_overrides_;
    static std::unordered_map<std::string, FunctionImportanceOverride> function_overrides_;
    static std::unordered_map<std::string, ApplicationImportanceConfig> application_configs_;

public:
    // =================================================================================
    // DEFAULT IMPORTANCE MAPPING
    // =================================================================================

    /**
     * @brief Initialize default importance mapping for message types
     * Should be called once at application startup
     */
    static void InitializeDefaultMapping();

    /**
     * @brief Set default importance for a message type
     * @param type Message type
     * @param importance Importance level
     */
    static void SetDefaultImportance(LogMessageType type, MessageImportance importance);

    /**
     * @brief Get default importance for a message type
     * @param type Message type
     * @return Default importance level
     */
    static MessageImportance GetDefaultImportance(LogMessageType type);

    /**
     * @brief Get all default type mappings
     * @return Vector of type-importance pairs
     */
    static std::vector<std::pair<LogMessageType, MessageImportance>> GetAllDefaultMappings();

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
    static uint32_t AddComponentOverride(const std::string& component_pattern,
                                        MessageImportance importance,
                                        bool use_regex = false,
                                        const std::string& reason = "");

    /**
     * @brief Remove component importance override
     * @param override_id Override ID to remove
     * @return true if override was found and removed
     */
    static bool RemoveComponentOverride(uint32_t override_id);

    /**
     * @brief Update component importance override
     * @param override_id Override ID to update
     * @param importance New importance level
     * @param reason New reason (optional)
     * @return true if override was found and updated
     */
    static bool UpdateComponentOverride(uint32_t override_id,
                                       MessageImportance importance,
                                       const std::string& reason = "");

    /**
     * @brief Find component override for a component name
     * @param component_name Component name to match
     * @return Pointer to override or nullptr if not found
     */
    static const ComponentImportanceOverride* FindComponentOverride(const std::string& component_name);

    /**
     * @brief Get all component overrides
     * @return Vector of all component overrides
     */
    static std::vector<ComponentImportanceOverride> GetAllComponentOverrides();

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
    static uint32_t AddFunctionOverride(const std::string& function_pattern,
                                       MessageImportance importance,
                                       bool use_regex = false,
                                       const std::string& reason = "");

    /**
     * @brief Remove function importance override
     * @param override_id Override ID to remove
     * @return true if override was found and removed
     */
    static bool RemoveFunctionOverride(uint32_t override_id);

    /**
     * @brief Update function importance override
     * @param override_id Override ID to update
     * @param importance New importance level
     * @param reason New reason (optional)
     * @return true if override was found and updated
     */
    static bool UpdateFunctionOverride(uint32_t override_id,
                                      MessageImportance importance,
                                      const std::string& reason = "");

    /**
     * @brief Find function override for a function name
     * @param function_name Function name to match
     * @return Pointer to override or nullptr if not found
     */
    static const FunctionImportanceOverride* FindFunctionOverride(const std::string& function_name);

    /**
     * @brief Get all function overrides
     * @return Vector of all function overrides
     */
    static std::vector<FunctionImportanceOverride> GetAllFunctionOverrides();

    // =================================================================================
    // APPLICATION-SPECIFIC CONFIGURATION
    // =================================================================================

    /**
     * @brief Create application-specific importance configuration
     * @param application_name Name of the application
     * @return Configuration ID
     */
    static uint32_t CreateApplicationConfig(const std::string& application_name);

    /**
     * @brief Load application importance configuration
     * @param application_name Application name
     * @param config Configuration data to load
     * @return true if loaded successfully
     */
    static bool LoadApplicationConfig(const std::string& application_name,
                                     const ApplicationImportanceConfig& config);

    /**
     * @brief Save application importance configuration
     * @param application_name Application name
     * @return Configuration data
     */
    static ApplicationImportanceConfig SaveApplicationConfig(const std::string& application_name);

    /**
     * @brief Delete application configuration
     * @param application_name Application name
     * @return true if configuration was found and deleted
     */
    static bool DeleteApplicationConfig(const std::string& application_name);

    // =================================================================================
    // IMPORTANCE RESOLUTION (CORE LOGIC)
    // =================================================================================

    /**
     * @brief Resolve message importance using hierarchy: Function > Component > Type > Default
     * @param message Message data to analyze
     * @param context Additional context for resolution
     * @return Resolution result with final importance and reasoning
     */
    static ImportanceResolutionResult ResolveMessageImportance(
        const LogMessageData& message,
        const ImportanceResolutionContext& context);

    /**
     * @brief Resolve importance for message type only (no overrides)
     * @param type Message type
     * @return Importance level
     */
    static MessageImportance ResolveTypeImportance(LogMessageType type);

    /**
     * @brief Resolve importance with component override consideration
     * @param type Message type
     * @param component Component name
     * @return Importance level
     */
    static MessageImportance ResolveComponentImportance(LogMessageType type,
                                                       const std::string& component);

    /**
     * @brief Resolve importance with function override consideration
     * @param type Message type
     * @param component Component name
     * @param function Function name
     * @return Importance level
     */
    static MessageImportance ResolveFunctionImportance(LogMessageType type,
                                                      const std::string& component,
                                                      const std::string& function);

    /**
     * @brief Resolve importance with full context consideration
     * @param type Message type
     * @param component Component name
     * @param function Function name
     * @param application_name Application name
     * @param system_load Current system load (0-100)
     * @param error_rate Current error rate
     * @return Importance level
     */
    static MessageImportance ResolveContextualImportance(
        LogMessageType type,
        const std::string& component,
        const std::string& function,
        const std::string& application_name,
        DWORD system_load = 0,
        DWORD error_rate = 0);

    // =================================================================================
    // PATTERN MATCHING
    // =================================================================================

    /**
     * @brief Check if component name matches override pattern
     * @param component_name Component name to check
     * @param pattern Pattern to match against (supports wildcards)
     * @param use_regex Whether pattern is regex
     * @return true if component matches pattern
     */
    static bool MatchesComponentPattern(const std::string& component_name,
                                       const std::string& pattern,
                                       bool use_regex = false);

    /**
     * @brief Check if function name matches override pattern
     * @param function_name Function name to check
     * @param pattern Pattern to match against (supports wildcards)
     * @param use_regex Whether pattern is regex
     * @return true if function matches pattern
     */
    static bool MatchesFunctionPattern(const std::string& function_name,
                                      const std::string& pattern,
                                      bool use_regex = false);

    /**
     * @brief Convert wildcard pattern to regex
     * @param wildcard_pattern Pattern with wildcards (*, ?)
     * @return Regex pattern string
     */
    static std::string WildcardToRegex(const std::string& wildcard_pattern);

    // =================================================================================
    // BATCH OPERATIONS
    // =================================================================================

    /**
     * @brief Resolve importance for multiple messages
     * @param messages Vector of messages to analyze
     * @param context Resolution context
     * @return Vector of resolution results
     */
    static std::vector<ImportanceResolutionResult> ResolveBatchImportance(
        const std::vector<LogMessageData>& messages,
        const ImportanceResolutionContext& context);

    /**
     * @brief Filter messages by minimum importance
     * @param messages Vector of messages to filter
     * @param min_importance Minimum importance level
     * @return Vector of messages meeting importance criteria
     */
    static std::vector<LogMessageData> FilterByImportance(
        const std::vector<LogMessageData>& messages,
        MessageImportance min_importance);

    /**
     * @brief Count messages by importance level
     * @param messages Vector of messages to analyze
     * @return Vector of pairs (importance, count)
     */
    static std::vector<std::pair<MessageImportance, size_t>> CountByImportance(
        const std::vector<LogMessageData>& messages);

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
    static bool ShouldPersistMessage(const LogMessageData& message,
                                    const ImportanceResolutionContext& context,
                                    MessageImportance min_persistence_importance);

    /**
     * @brief Determine if message should be persisted based on component
     * @param component Component name
     * @param message_type Message type
     * @param system_load Current system load
     * @return true if message should be persisted
     */
    static bool ShouldPersistByComponent(const std::string& component,
                                        LogMessageType message_type,
                                        DWORD system_load = 0);

    /**
     * @brief Determine if message should be persisted based on system conditions
     * @param message_type Message type
     * @param system_load Current system load (0-100)
     * @param error_rate Current error rate
     * @param is_emergency_mode Whether system is in emergency mode
     * @return true if message should be persisted
     */
    static bool ShouldPersistBySystemConditions(LogMessageType message_type,
                                               DWORD system_load,
                                               DWORD error_rate,
                                               bool is_emergency_mode);

    // =================================================================================
    // CONFIGURATION MANAGEMENT
    // =================================================================================

    /**
     * @brief Load importance configuration from file
     * @param config_file Path to configuration file
     * @return true if configuration loaded successfully
     */
    static bool LoadConfigurationFromFile(const std::string& config_file);

    /**
     * @brief Save importance configuration to file
     * @param config_file Path to save configuration file
     * @return true if configuration saved successfully
     */
    static bool SaveConfigurationToFile(const std::string& config_file);

    /**
     * @brief Reset all importance mappings to defaults
     */
    static void ResetToDefaults();

    /**
     * @brief Validate importance configuration
     * @param config Configuration to validate
     * @return true if configuration is valid
     */
    static bool ValidateConfiguration(const ApplicationImportanceConfig& config);

    // =================================================================================
    // STATISTICS AND ANALYSIS
    // =================================================================================

    /**
     * @brief Get importance mapping statistics
     * @return String containing mapping statistics
     */
    static std::string GetMappingStatistics();

    /**
     * @brief Get override usage statistics
     * @return String containing override statistics
     */
    static std::string GetOverrideStatistics();

    /**
     * @brief Analyze importance distribution in message batch
     * @param messages Vector of messages to analyze
     * @return Analysis results as formatted string
     */
    static std::string AnalyzeImportanceDistribution(const std::vector<LogMessageData>& messages);

    /**
     * @brief Get most common component overrides
     * @param max_results Maximum number of results to return
     * @return Vector of component-override pairs
     */
    static std::vector<std::pair<std::string, MessageImportance>> GetMostUsedComponentOverrides(
        size_t max_results = 10);

    /**
     * @brief Get most common function overrides
     * @param max_results Maximum number of results to return
     * @return Vector of function-override pairs
     */
    static std::vector<std::pair<std::string, MessageImportance>> GetMostUsedFunctionOverrides(
        size_t max_results = 10);

private:
    // Private helper methods
    static MessageImportance ApplyOverrides(LogMessageType type,
                                           const std::string& component,
                                           const std::string& function);
    static bool PatternMatches(const std::string& text, const std::string& pattern, bool use_regex);
    static std::regex CompilePattern(const std::string& pattern, bool use_regex);
    static ImportanceResolutionResult CreateResolutionResult(MessageImportance importance,
                                                           const std::string& override_type,
                                                           const std::string& reason);
    static uint64_t GetCurrentTimeMicroseconds();
    static void InitializeDefaultMappings();
    static bool IsInitialized();
};

#endif // __ASFM_LOGGER_IMPORTANCE_TOOLBOX_HPP__