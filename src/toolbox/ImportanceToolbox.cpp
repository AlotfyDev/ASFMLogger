/**
 * ASFMLogger Importance Framework Toolbox Implementation
 *
 * Static methods for importance mapping and resolution following toolbox architecture.
 * Pure functions for smart message classification and contextual importance decisions.
 */

#include "ImportanceToolbox.hpp"
#include "../ASFMLoggerCore.hpp"
#include "../structs/ImportanceConfiguration.hpp"
#include "../structs/LogDataStructures.hpp"
#include "TimestampToolbox.hpp"
#include "LoggerInstanceToolbox.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <chrono>

// Static variables
std::unordered_map<LogMessageType, MessageImportance> ImportanceToolbox::default_type_mapping_;
std::unordered_map<std::string, ComponentImportanceOverride> ImportanceToolbox::component_overrides_;
std::unordered_map<std::string, FunctionImportanceOverride> ImportanceToolbox::function_overrides_;
std::unordered_map<std::string, ApplicationImportanceConfig> ImportanceToolbox::application_configs_;

// =====================================================================================
// DEFAULT IMPORTANCE MAPPING
// =====================================================================================

void ImportanceToolbox::InitializeDefaultMapping() {
    if (IsInitialized()) return;

    default_type_mapping_[LogMessageType::TRACE] = MessageImportance::LOW;
    default_type_mapping_[LogMessageType::DEBUG] = MessageImportance::LOW;
    default_type_mapping_[LogMessageType::INFO] = MessageImportance::MEDIUM;
    default_type_mapping_[LogMessageType::WARN] = MessageImportance::HIGH;
    default_type_mapping_[LogMessageType::ERR] = MessageImportance::CRITICAL;
    default_type_mapping_[LogMessageType::CRITICAL_LOG] = MessageImportance::CRITICAL;
}

void ImportanceToolbox::SetDefaultImportance(LogMessageType type, MessageImportance importance) {
    default_type_mapping_[type] = importance;
}

MessageImportance ImportanceToolbox::GetDefaultImportance(LogMessageType type) {
    auto it = default_type_mapping_.find(type);
    return (it != default_type_mapping_.end()) ? it->second : MessageImportance::LOW;
}

std::vector<std::pair<LogMessageType, MessageImportance>> ImportanceToolbox::GetAllDefaultMappings() {
    std::vector<std::pair<LogMessageType, MessageImportance>> result;
    for (const auto& pair : default_type_mapping_) {
        result.emplace_back(pair.first, pair.second);
    }
    return result;
}

// =====================================================================================
// COMPONENT IMPORTANCE OVERRIDES
// =====================================================================================

uint32_t ImportanceToolbox::AddComponentOverride(const std::string& component_pattern,
                                                MessageImportance importance,
                                                bool use_regex,
                                                const std::string& reason) {
    static uint32_t next_id = 1;

    ComponentImportanceOverride override;
    memset(&override, 0, sizeof(ComponentImportanceOverride));

    override.mapping_id = next_id++;
    strcpy_s(override.component_pattern, component_pattern.c_str());
    override.importance = importance;
    override.use_regex = use_regex;
    override.created_time = LoggerInstanceToolbox::GetCurrentTimestamp();
    strcpy_s(override.created_by, "System");
    strcpy_s(override.reason, reason.c_str());

    std::string key = component_pattern + (use_regex ? "_regex" : "_wildcard");
    component_overrides_[key] = override;

    return override.mapping_id;
}

bool ImportanceToolbox::RemoveComponentOverride(uint32_t override_id) {
    for (auto it = component_overrides_.begin(); it != component_overrides_.end(); ++it) {
        if (it->second.mapping_id == override_id) {
            component_overrides_.erase(it);
            return true;
        }
    }
    return false;
}

bool ImportanceToolbox::UpdateComponentOverride(uint32_t override_id,
                                               MessageImportance importance,
                                               const std::string& reason) {
    for (auto& pair : component_overrides_) {
        if (pair.second.mapping_id == override_id) {
            pair.second.importance = importance;
            strcpy_s(pair.second.reason, reason.c_str());
            return true;
        }
    }
    return false;
}

const ComponentImportanceOverride* ImportanceToolbox::FindComponentOverride(const std::string& component_name) {
    for (const auto& pair : component_overrides_) {
        if (MatchesComponentPattern(component_name, pair.first.substr(0, pair.first.find_last_of("_")))) {
            return &pair.second;
        }
    }
    return nullptr;
}

std::vector<ComponentImportanceOverride> ImportanceToolbox::GetAllComponentOverrides() {
    std::vector<ComponentImportanceOverride> result;
    for (const auto& pair : component_overrides_) {
        result.push_back(pair.second);
    }
    return result;
}

// =====================================================================================
// FUNCTION IMPORTANCE OVERRIDES
// =====================================================================================

uint32_t ImportanceToolbox::AddFunctionOverride(const std::string& function_pattern,
                                               MessageImportance importance,
                                               bool use_regex,
                                               const std::string& reason) {
    static uint32_t next_id = 1;

    FunctionImportanceOverride override;
    memset(&override, 0, sizeof(FunctionImportanceOverride));

    override.mapping_id = next_id++;
    strcpy_s(override.function_pattern, function_pattern.c_str());
    override.importance = importance;
    override.use_regex = use_regex;
    override.created_time = LoggerInstanceToolbox::GetCurrentTimestamp();
    strcpy_s(override.created_by, "System");
    strcpy_s(override.reason, reason.c_str());

    std::string key = function_pattern + (use_regex ? "_regex" : "_wildcard");
    function_overrides_[key] = override;

    return override.mapping_id;
}

bool ImportanceToolbox::RemoveFunctionOverride(uint32_t override_id) {
    for (auto it = function_overrides_.begin(); it != function_overrides_.end(); ++it) {
        if (it->second.mapping_id == override_id) {
            function_overrides_.erase(it);
            return true;
        }
    }
    return false;
}

bool ImportanceToolbox::UpdateFunctionOverride(uint32_t override_id,
                                              MessageImportance importance,
                                              const std::string& reason) {
    for (auto& pair : function_overrides_) {
        if (pair.second.mapping_id == override_id) {
            pair.second.importance = importance;
            strcpy_s(pair.second.reason, reason.c_str());
            return true;
        }
    }
    return false;
}

const FunctionImportanceOverride* ImportanceToolbox::FindFunctionOverride(const std::string& function_name) {
    for (const auto& pair : function_overrides_) {
        if (MatchesFunctionPattern(function_name, pair.first.substr(0, pair.first.find_last_of("_")))) {
            return &pair.second;
        }
    }
    return nullptr;
}

std::vector<FunctionImportanceOverride> ImportanceToolbox::GetAllFunctionOverrides() {
    std::vector<FunctionImportanceOverride> result;
    for (const auto& pair : function_overrides_) {
        result.push_back(pair.second);
    }
    return result;
}

// =====================================================================================
// APPLICATION-SPECIFIC CONFIGURATION
// =====================================================================================

uint32_t ImportanceToolbox::CreateApplicationConfig(const std::string& application_name) {
    static uint32_t next_id = 1;

    ApplicationImportanceConfig config;
    memset(&config, 0, sizeof(ApplicationImportanceConfig));

    config.config_id = next_id++;
    strcpy_s(config.application_name, application_name.c_str());
    config.is_active = true;
    config.last_modified = LoggerInstanceToolbox::GetCurrentTimestamp();
    strcpy_s(config.modified_by, "System");

    application_configs_[application_name] = config;
    return config.config_id;
}

bool ImportanceToolbox::LoadApplicationConfig(const std::string& application_name,
                                             const ApplicationImportanceConfig& config) {
    if (application_configs_.find(application_name) == application_configs_.end()) {
        return false;
    }

    application_configs_[application_name] = config;
    return true;
}

ApplicationImportanceConfig ImportanceToolbox::SaveApplicationConfig(const std::string& application_name) {
    auto it = application_configs_.find(application_name);
    if (it != application_configs_.end()) {
        return it->second;
    }

    ApplicationImportanceConfig empty_config;
    memset(&empty_config, 0, sizeof(ApplicationImportanceConfig));
    return empty_config;
}

bool ImportanceToolbox::DeleteApplicationConfig(const std::string& application_name) {
    auto it = application_configs_.find(application_name);
    if (it != application_configs_.end()) {
        application_configs_.erase(it);
        return true;
    }
    return false;
}

// =====================================================================================
// IMPORTANCE RESOLUTION (CORE LOGIC)
// =====================================================================================

ImportanceResolutionResult ImportanceToolbox::ResolveMessageImportance(
    const LogMessageData& message,
    const ImportanceResolutionContext& context) {

    uint64_t start_time = GetCurrentTimeMicroseconds();

    MessageImportance importance = ApplyOverrides(
        message.type,
        context.component,
        context.function);

    uint64_t end_time = GetCurrentTimeMicroseconds();
    uint32_t resolution_time = static_cast<uint32_t>(end_time - start_time);

    return CreateResolutionResult(importance, "NONE", "Default resolution");
}

MessageImportance ImportanceToolbox::ResolveTypeImportance(LogMessageType type) {
    return GetDefaultImportance(type);
}

MessageImportance ImportanceToolbox::ResolveComponentImportance(LogMessageType type,
                                                              const std::string& component) {
    // Check for component override first
    const ComponentImportanceOverride* component_override = FindComponentOverride(component);
    if (component_override) {
        return component_override->importance;
    }

    // Fall back to type default
    return GetDefaultImportance(type);
}

MessageImportance ImportanceToolbox::ResolveFunctionImportance(LogMessageType type,
                                                             const std::string& component,
                                                             const std::string& function) {
    // Check for function override first
    const FunctionImportanceOverride* function_override = FindFunctionOverride(function);
    if (function_override) {
        return function_override->importance;
    }

    // Check for component override
    const ComponentImportanceOverride* component_override = FindComponentOverride(component);
    if (component_override) {
        return component_override->importance;
    }

    // Fall back to type default
    return GetDefaultImportance(type);
}

MessageImportance ImportanceToolbox::ResolveContextualImportance(
    LogMessageType type,
    const std::string& component,
    const std::string& function,
    const std::string& application_name,
    DWORD system_load,
    DWORD error_rate) {

    MessageImportance base_importance = ResolveFunctionImportance(type, component, function);

    // Apply contextual adjustments
    if (system_load > 80) {
        // Increase importance during high system load
        if (base_importance < MessageImportance::HIGH) {
            base_importance = static_cast<MessageImportance>(static_cast<int>(base_importance) + 1);
        }
    }

    if (error_rate > 10) {
        // Increase importance during high error rates
        if (base_importance < MessageImportance::CRITICAL) {
            base_importance = static_cast<MessageImportance>(static_cast<int>(base_importance) + 1);
        }
    }

    return base_importance;
}

// =====================================================================================
// PATTERN MATCHING
// =====================================================================================

bool ImportanceToolbox::MatchesComponentPattern(const std::string& component_name,
                                               const std::string& pattern,
                                               bool use_regex) {
    return PatternMatches(component_name, pattern, use_regex);
}

bool ImportanceToolbox::MatchesFunctionPattern(const std::string& function_name,
                                              const std::string& pattern,
                                              bool use_regex) {
    return PatternMatches(function_name, pattern, use_regex);
}

std::string ImportanceToolbox::WildcardToRegex(const std::string& wildcard_pattern) {
    std::string regex_pattern = wildcard_pattern;
    size_t pos = 0;

    // Replace * with .*
    while ((pos = regex_pattern.find("*", pos)) != std::string::npos) {
        regex_pattern.replace(pos, 1, ".*");
        pos += 2;
    }

    // Replace ? with .
    pos = 0;
    while ((pos = regex_pattern.find("?", pos)) != std::string::npos) {
        regex_pattern.replace(pos, 1, ".");
        pos += 1;
    }

    return regex_pattern;
}

// =====================================================================================
// BATCH OPERATIONS
// =====================================================================================

std::vector<ImportanceResolutionResult> ImportanceToolbox::ResolveBatchImportance(
    const std::vector<LogMessageData>& messages,
    const ImportanceResolutionContext& context) {

    std::vector<ImportanceResolutionResult> results;
    results.reserve(messages.size());

    for (const auto& message : messages) {
        results.push_back(ResolveMessageImportance(message, context));
    }

    return results;
}

std::vector<LogMessageData> ImportanceToolbox::FilterByImportance(
    const std::vector<LogMessageData>& messages,
    MessageImportance min_importance) {

    std::vector<LogMessageData> result;
    std::copy_if(messages.begin(), messages.end(), std::back_inserter(result),
                 [min_importance](const LogMessageData& message) {
                     return ResolveTypeImportance(message.type) >= min_importance;
                 });

    return result;
}

std::vector<std::pair<MessageImportance, size_t>> ImportanceToolbox::CountByImportance(
    const std::vector<LogMessageData>& messages) {

    std::unordered_map<MessageImportance, size_t> counts;
    for (const auto& message : messages) {
        MessageImportance importance = ResolveTypeImportance(message.type);
        counts[importance]++;
    }

    std::vector<std::pair<MessageImportance, size_t>> result;
    for (const auto& pair : counts) {
        result.emplace_back(pair.first, pair.second);
    }

    std::sort(result.begin(), result.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    return result;
}

// =====================================================================================
// PERSISTENCE DECISIONS
// =====================================================================================

bool ImportanceToolbox::ShouldPersistMessage(const LogMessageData& message,
                                            const ImportanceResolutionContext& context,
                                            MessageImportance min_persistence_importance) {
    MessageImportance importance = ResolveMessageImportance(message, context).final_importance;
    return importance >= min_persistence_importance;
}

bool ImportanceToolbox::ShouldPersistByComponent(const std::string& component,
                                                LogMessageType message_type,
                                                DWORD system_load) {
    MessageImportance importance = ResolveComponentImportance(message_type, component);

    // Always persist critical messages
    if (importance >= MessageImportance::CRITICAL) {
        return true;
    }

    // Persist high importance messages during high system load
    if (system_load > 70 && importance >= MessageImportance::HIGH) {
        return true;
    }

    // Persist medium importance messages during very high system load
    if (system_load > 90 && importance >= MessageImportance::MEDIUM) {
        return true;
    }

    return false;
}

bool ImportanceToolbox::ShouldPersistBySystemConditions(LogMessageType message_type,
                                                      DWORD system_load,
                                                      DWORD error_rate,
                                                      bool is_emergency_mode) {
    if (is_emergency_mode) {
        return true; // Persist everything in emergency mode
    }

    MessageImportance default_importance = GetDefaultImportance(message_type);

    // Critical messages always persist
    if (default_importance >= MessageImportance::CRITICAL) {
        return true;
    }

    // High importance messages persist during high load or error rates
    if (default_importance >= MessageImportance::HIGH) {
        return system_load > 80 || error_rate > 5;
    }

    // Medium importance messages persist during extreme conditions
    if (default_importance >= MessageImportance::MEDIUM) {
        return system_load > 95 || error_rate > 10;
    }

    return false;
}

// =====================================================================================
// CONFIGURATION MANAGEMENT
// =====================================================================================

bool ImportanceToolbox::LoadConfigurationFromFile(const std::string& config_file) {
    std::ifstream file(config_file);
    if (!file.is_open()) {
        return false;
    }

    // Simple configuration loading (could be enhanced)
    std::string line;
    while (std::getline(file, line)) {
        // Parse configuration lines
        // This is a simplified implementation
    }

    return true;
}

bool ImportanceToolbox::SaveConfigurationToFile(const std::string& config_file) {
    std::ofstream file(config_file);
    if (!file.is_open()) {
        return false;
    }

    // Write current configuration
    file << "# ASFMLogger Importance Configuration" << std::endl;
    file << "# Generated: " << LoggerInstanceToolbox::GetCurrentTimestamp() << std::endl;

    for (const auto& pair : default_type_mapping_) {
        file << "TYPE:" << static_cast<int>(pair.first) << "=" << static_cast<int>(pair.second) << std::endl;
    }

    return true;
}

void ImportanceToolbox::ResetToDefaults() {
    default_type_mapping_.clear();
    component_overrides_.clear();
    function_overrides_.clear();
    application_configs_.clear();
    InitializeDefaultMapping();
}

bool ImportanceToolbox::ValidateConfiguration(const ApplicationImportanceConfig& config) {
    return config.config_id != 0 &&
           strlen(config.application_name) > 0 &&
           config.is_active;
}

// =====================================================================================
// STATISTICS AND ANALYSIS
// =====================================================================================

std::string ImportanceToolbox::GetMappingStatistics() {
    std::ostringstream oss;
    oss << "Default Mappings: " << default_type_mapping_.size() << std::endl;
    oss << "Component Overrides: " << component_overrides_.size() << std::endl;
    oss << "Function Overrides: " << function_overrides_.size() << std::endl;
    oss << "Application Configs: " << application_configs_.size() << std::endl;
    return oss.str();
}

std::string ImportanceToolbox::GetOverrideStatistics() {
    std::ostringstream oss;
    oss << "Component Overrides: " << component_overrides_.size() << std::endl;
    oss << "Function Overrides: " << function_overrides_.size() << std::endl;

    size_t regex_overrides = 0;
    for (const auto& pair : component_overrides_) {
        if (pair.second.use_regex) regex_overrides++;
    }

    oss << "Regex Component Overrides: " << regex_overrides << std::endl;
    return oss.str();
}

std::string ImportanceToolbox::AnalyzeImportanceDistribution(const std::vector<LogMessageData>& messages) {
    auto counts = CountByImportance(messages);

    std::ostringstream oss;
    oss << "Importance Distribution:" << std::endl;

    for (const auto& pair : counts) {
        oss << MessageImportanceToString(pair.first) << ": " << pair.second << " messages" << std::endl;
    }

    return oss.str();
}

std::vector<std::pair<std::string, MessageImportance>> ImportanceToolbox::GetMostUsedComponentOverrides(
    size_t max_results) {
    std::vector<std::pair<std::string, MessageImportance>> result;

    for (const auto& pair : component_overrides_) {
        std::string pattern = pair.first.substr(0, pair.first.find_last_of("_"));
        result.emplace_back(pattern, pair.second.importance);
    }

    std::sort(result.begin(), result.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    if (result.size() > max_results) {
        result.resize(max_results);
    }

    return result;
}

std::vector<std::pair<std::string, MessageImportance>> ImportanceToolbox::GetMostUsedFunctionOverrides(
    size_t max_results) {
    std::vector<std::pair<std::string, MessageImportance>> result;

    for (const auto& pair : function_overrides_) {
        std::string pattern = pair.first.substr(0, pair.first.find_last_of("_"));
        result.emplace_back(pattern, pair.second.importance);
    }

    std::sort(result.begin(), result.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    if (result.size() > max_results) {
        result.resize(max_results);
    }

    return result;
}

// =====================================================================================
// PRIVATE HELPER METHODS
// =====================================================================================

MessageImportance ImportanceToolbox::ApplyOverrides(LogMessageType type,
                                                  const std::string& component,
                                                  const std::string& function) {
    // Check function override first (highest priority)
    const FunctionImportanceOverride* function_override = FindFunctionOverride(function);
    if (function_override) {
        return function_override->importance;
    }

    // Check component override
    const ComponentImportanceOverride* component_override = FindComponentOverride(component);
    if (component_override) {
        return component_override->importance;
    }

    // Fall back to type default
    return GetDefaultImportance(type);
}

bool ImportanceToolbox::PatternMatches(const std::string& text, const std::string& pattern, bool use_regex) {
    if (use_regex) {
        try {
            std::regex regex_pattern = CompilePattern(pattern, true);
            return std::regex_match(text, regex_pattern);
        } catch (const std::regex_error&) {
            return false;
        }
    } else {
        // Simple wildcard matching
        std::string regex_pattern = WildcardToRegex(pattern);
        try {
            std::regex compiled_pattern(regex_pattern);
            return std::regex_match(text, compiled_pattern);
        } catch (const std::regex_error&) {
            return false;
        }
    }
}

std::regex ImportanceToolbox::CompilePattern(const std::string& pattern, bool use_regex) {
    if (use_regex) {
        return std::regex(pattern);
    } else {
        std::string regex_pattern = WildcardToRegex(pattern);
        return std::regex(regex_pattern);
    }
}

ImportanceResolutionResult ImportanceToolbox::CreateResolutionResult(MessageImportance importance,
                                                                   const std::string& override_type,
                                                                   const std::string& reason) {
    ImportanceResolutionResult result;
    memset(&result, 0, sizeof(ImportanceResolutionResult));

    result.final_importance = importance;
    result.was_overridden = (override_type != "NONE");
    strcpy_s(result.applied_override_type, override_type.c_str());
    strcpy_s(result.override_reason, reason.c_str());
    result.resolved_at = LoggerInstanceToolbox::GetCurrentTimestamp();

    return result;
}

uint64_t ImportanceToolbox::GetCurrentTimeMicroseconds() {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(
        now.time_since_epoch()).count();
}

void ImportanceToolbox::InitializeDefaultMappings() {
    InitializeDefaultMapping();
}

bool ImportanceToolbox::IsInitialized() {
    return !default_type_mapping_.empty();
}