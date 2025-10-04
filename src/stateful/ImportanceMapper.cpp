/**
 * ASFMLogger ImportanceMapper Implementation
 *
 * Stateful wrapper implementation for importance configuration management.
 * Provides thread-safe object-oriented interface using ImportanceToolbox internally.
 */

#include "stateful/ImportanceMapper.hpp"
#include "toolbox/ImportanceToolbox.hpp"
#include "structs/ImportanceConfiguration.hpp"
#include <sstream>
#include <algorithm>
#include <mutex>
#include <chrono>
#include <cstring>
#ifdef _WIN32
#include <windows.h>
#endif

// =====================================================================================
// CONSTRUCTORS AND DESTRUCTOR
// =====================================================================================

ImportanceMapper::ImportanceMapper()
    : application_name_(), config_(), mapper_mutex_() {
    // Initialize with empty application name
    // Configuration will be loaded when application name is set
}

ImportanceMapper::ImportanceMapper(const std::string& application_name)
    : application_name_(application_name), config_(), mapper_mutex_() {

    // Validate input
    if (application_name.empty()) {
        throw std::invalid_argument("Application name cannot be empty");
    }

    // Create application configuration using ToolBox
    ImportanceToolbox::CreateApplicationConfig(application_name);

    // Initialize with default configuration
    ImportanceToolbox::InitializeDefaultMapping();
}

ImportanceMapper::ImportanceMapper(const std::string& application_name,
                                   const ApplicationImportanceConfig& config)
    : application_name_(application_name), config_(config), mapper_mutex_() {

    // Validate inputs
    if (application_name.empty()) {
        throw std::invalid_argument("Application name cannot be empty");
    }

    // Validate configuration
    if (!ImportanceToolbox::ValidateConfiguration(config)) {
        throw std::invalid_argument("Invalid importance configuration provided");
    }

    // Load configuration using ToolBox
    if (!ImportanceToolbox::LoadApplicationConfig(application_name, config)) {
        throw std::runtime_error("Failed to load importance configuration");
    }
}

ImportanceMapper::ImportanceMapper(const ImportanceMapper& other)
    : application_name_(other.application_name_), config_(other.config_), mapper_mutex_() {
    // Copy constructor with proper mutex handling
    std::lock_guard<std::mutex> lock(other.mapper_mutex_);
    application_name_ = other.application_name_;
    config_ = other.config_;
}

ImportanceMapper::ImportanceMapper(ImportanceMapper&& other) noexcept
    : application_name_(std::move(other.application_name_)),
      config_(std::move(other.config_)),
      mapper_mutex_() {
    // Move constructor - data is already moved
}

ImportanceMapper& ImportanceMapper::operator=(const ImportanceMapper& other) {
    if (this != &other) {
        std::lock_guard<std::mutex> this_lock(mapper_mutex_);
        std::lock_guard<std::mutex> other_lock(other.mapper_mutex_);

        application_name_ = other.application_name_;
        config_ = other.config_;
    }
    return *this;
}

ImportanceMapper& ImportanceMapper::operator=(ImportanceMapper&& other) noexcept {
    if (this != &other) {
        std::lock_guard<std::mutex> this_lock(mapper_mutex_);

        application_name_ = std::move(other.application_name_);
        config_ = std::move(other.config_);
        // other data is left in valid but unspecified state
    }
    return *this;
}

// =====================================================================================
// APPLICATION MANAGEMENT
// =====================================================================================

void ImportanceMapper::setApplicationName(const std::string& application_name) {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    if (application_name.empty()) {
        throw std::invalid_argument("Application name cannot be empty");
    }

    // Delete old configuration if it exists
    if (!application_name_.empty()) {
        ImportanceToolbox::DeleteApplicationConfig(application_name_);
    }

    // Set new application name
    application_name_ = application_name;

    // Create new configuration
    ImportanceToolbox::CreateApplicationConfig(application_name);
}

// =====================================================================================
// TYPE-BASED IMPORTANCE MAPPING
// =====================================================================================

bool ImportanceMapper::setTypeImportance(LogMessageType type, MessageImportance importance,
                                        const std::string& reason) {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    if (application_name_.empty()) {
        return false;
    }

    // Use ToolBox for type importance setting
    ImportanceToolbox::SetDefaultImportance(type, importance);

    // Update configuration if provided
    if (!reason.empty()) {
        // Store in configuration - find the appropriate type mapping
        for (int i = 0; i < 6; ++i) {
            if (config_.type_mappings[i].message_type == type) {
                config_.type_mappings[i].default_importance = importance;
                break;
            }
        }
    }

    return true;
}

MessageImportance ImportanceMapper::getTypeImportance(LogMessageType type) const {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    // Use ToolBox for type importance resolution
    return ImportanceToolbox::ResolveTypeImportance(type);
}

bool ImportanceMapper::resetTypeImportance(LogMessageType type) {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    if (application_name_.empty()) {
        return false;
    }

    // Reset to default using ToolBox
    ImportanceToolbox::SetDefaultImportance(type, static_cast<MessageImportance>(type));

    // Update configuration - find and reset the appropriate type mapping
    for (int i = 0; i < 6; ++i) {
        if (config_.type_mappings[i].message_type == type) {
            config_.type_mappings[i].default_importance = static_cast<MessageImportance>(type);
            config_.type_mappings[i].use_override = false;
            break;
        }
    }

    return true;
}

std::vector<std::pair<LogMessageType, MessageImportance>> ImportanceMapper::getAllTypeMappings() const {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    // Use ToolBox to get all default mappings
    return ImportanceToolbox::GetAllDefaultMappings();
}

// =====================================================================================
// COMPONENT IMPORTANCE OVERRIDES
// =====================================================================================

uint32_t ImportanceMapper::addComponentOverride(const std::string& component_pattern,
                                               MessageImportance importance,
                                               bool use_regex,
                                               const std::string& reason) {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    if (application_name_.empty() || component_pattern.empty()) {
        return 0;
    }

    // Use ToolBox to add component override
    return ImportanceToolbox::AddComponentOverride(component_pattern, importance, use_regex, reason);
}

bool ImportanceMapper::removeComponentOverride(uint32_t override_id) {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    // Use ToolBox to remove component override
    return ImportanceToolbox::RemoveComponentOverride(override_id);
}

bool ImportanceMapper::updateComponentOverride(uint32_t override_id,
                                              MessageImportance importance,
                                              const std::string& reason) {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    // Use ToolBox to update component override
    return ImportanceToolbox::UpdateComponentOverride(override_id, importance, reason);
}

std::vector<ComponentImportanceOverride> ImportanceMapper::getComponentOverrides() const {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    // Use ToolBox to get all component overrides
    return ImportanceToolbox::GetAllComponentOverrides();
}

const ComponentImportanceOverride* ImportanceMapper::findComponentOverride(const std::string& component_name) const {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    // Use ToolBox to find component override
    return ImportanceToolbox::FindComponentOverride(component_name);
}

// =====================================================================================
// FUNCTION IMPORTANCE OVERRIDES
// =====================================================================================

uint32_t ImportanceMapper::addFunctionOverride(const std::string& function_pattern,
                                              MessageImportance importance,
                                              bool use_regex,
                                              const std::string& reason) {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    if (application_name_.empty() || function_pattern.empty()) {
        return 0;
    }

    // Use ToolBox to add function override
    return ImportanceToolbox::AddFunctionOverride(function_pattern, importance, use_regex, reason);
}

bool ImportanceMapper::removeFunctionOverride(uint32_t override_id) {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    // Use ToolBox to remove function override
    return ImportanceToolbox::RemoveFunctionOverride(override_id);
}

bool ImportanceMapper::updateFunctionOverride(uint32_t override_id,
                                             MessageImportance importance,
                                             const std::string& reason) {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    // Use ToolBox to update function override
    return ImportanceToolbox::UpdateFunctionOverride(override_id, importance, reason);
}

std::vector<FunctionImportanceOverride> ImportanceMapper::getFunctionOverrides() const {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    // Use ToolBox to get all function overrides
    return ImportanceToolbox::GetAllFunctionOverrides();
}

const FunctionImportanceOverride* ImportanceMapper::findFunctionOverride(const std::string& function_name) const {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    // Use ToolBox to find function override
    return ImportanceToolbox::FindFunctionOverride(function_name);
}

/**
 * @brief Resolve message importance using hierarchical override system
 * Resolution order: Function Override → Component Override → Type Default → System Default
 */
ImportanceResolutionResult ImportanceMapper::resolveMessageImportance(
    const LogMessageData& message,
    const ImportanceResolutionContext& context) const {

    std::lock_guard<std::mutex> lock(mapper_mutex_);

    ImportanceResolutionResult result = {};
    result.final_importance = MessageImportance::LOW;  // Default to lowest importance
    result.was_overridden = false;
    result.resolved_at = GetTickCount();
    result.override_reason[0] = '\0';
    result.applied_override_type[0] = '\0';

    // Get high-precision start time for resolution timing
    const auto start_time = std::chrono::high_resolution_clock::now();

    // Step 1: Check function-specific overrides (highest priority)
    if (strlen(context.function) > 0) {
        const FunctionImportanceOverride* func_override =
            ImportanceToolbox::FindFunctionOverride(context.function);

        if (func_override) {
            result.final_importance = func_override->importance;
            result.was_overridden = true;
            strcpy_s(result.applied_override_type, sizeof(result.applied_override_type), "FUNCTION");
            strcpy_s(result.override_reason, sizeof(result.override_reason), func_override->reason);
        }
    }

    // Step 2: If no function override, check component-specific overrides
    if (!result.was_overridden && strlen(context.component) > 0) {
        const ComponentImportanceOverride* comp_override =
            ImportanceToolbox::FindComponentOverride(context.component);

        if (comp_override) {
            result.final_importance = comp_override->importance;
            result.was_overridden = true;
            strcpy_s(result.applied_override_type, sizeof(result.applied_override_type), "COMPONENT");
            strcpy_s(result.override_reason, sizeof(result.override_reason), comp_override->reason);
        }
    }

    // Step 3: If no overrides, use type-based importance resolution
    if (!result.was_overridden) {
        result.final_importance = ImportanceToolbox::ResolveTypeImportance(context.message_type);
        strcpy_s(result.applied_override_type, sizeof(result.applied_override_type), "TYPE");
        strcpy_s(result.override_reason, sizeof(result.override_reason), "Default type mapping");
    }

    // Step 4: Apply contextual adjustments based on system state
    if (context.system_load > 80 || context.error_rate > 10) {
        // In high-load conditions, increase importance of critical messages
        if (result.final_importance >= MessageImportance::HIGH) {
            result.final_importance = MessageImportance::CRITICAL;
            strcat_s(result.override_reason, sizeof(result.override_reason), " [Elevated due to system load]");
        }
    }

    // Step 5: Emergency mode override
    if (context.is_emergency_mode) {
        // In emergency mode, elevate all messages
        if (result.final_importance < MessageImportance::HIGH) {
            result.final_importance = MessageImportance::CRITICAL;
            strcpy_s(result.override_reason, sizeof(result.override_reason), "Emergency mode override");
            result.was_overridden = true;
            strcpy_s(result.applied_override_type, sizeof(result.applied_override_type), "EMERGENCY");
        }
    }

    // Step 6: Calculate resolution time
    const auto end_time = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    result.resolution_time_microseconds = static_cast<uint32_t>(duration.count());

    return result;
}

// =====================================================================================
// PERSISTENCE DECISIONS
// =====================================================================================

// Persistence decision methods are already defined inline in the header file

// =====================================================================================
// CONFIGURATION MANAGEMENT
// =====================================================================================

bool ImportanceMapper::loadFromFile(const std::string& config_file) {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    if (application_name_.empty()) {
        return false;
    }

    // Use ToolBox to load configuration from file
    if (!ImportanceToolbox::LoadConfigurationFromFile(config_file)) {
        return false;
    }

    // Update local configuration
    config_ = ImportanceToolbox::SaveApplicationConfig(application_name_);

    return true;
}

bool ImportanceMapper::saveToFile(const std::string& config_file) const {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    if (application_name_.empty()) {
        return false;
    }

    // Use ToolBox to save configuration to file
    return ImportanceToolbox::SaveConfigurationToFile(config_file);
}

bool ImportanceMapper::resetToDefaults() {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    if (application_name_.empty()) {
        return false;
    }

    // Use ToolBox to reset to defaults
    ImportanceToolbox::ResetToDefaults();

    // Recreate application configuration
    ImportanceToolbox::CreateApplicationConfig(application_name_);

    return true;
}

bool ImportanceMapper::setConfiguration(const ApplicationImportanceConfig& config) {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    // Validate configuration
    if (!ImportanceToolbox::ValidateConfiguration(config)) {
        return false;
    }

    // Set local configuration
    config_ = config;

    // Load into ToolBox if application name is set
    if (!application_name_.empty()) {
        return ImportanceToolbox::LoadApplicationConfig(application_name_, config);
    }

    return true;
}

// =====================================================================================
// STATISTICS AND ANALYSIS
// =====================================================================================

// Statistics and analysis methods are already defined inline in the header file

// =====================================================================================
// BATCH OPERATIONS
// =====================================================================================

// Batch operation methods are already defined inline in the header file

// =====================================================================================
// UTILITY METHODS
// =====================================================================================

size_t ImportanceMapper::clearAllOverrides() {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    if (application_name_.empty()) {
        return 0;
    }

    // Get current override counts
    size_t component_count = getComponentOverrides().size();
    size_t function_count = getFunctionOverrides().size();

    // Reset to defaults (this clears overrides)
    ImportanceToolbox::ResetToDefaults();

    // Recreate application configuration
    ImportanceToolbox::CreateApplicationConfig(application_name_);

    return component_count + function_count;
}

size_t ImportanceMapper::clearComponentOverrides() {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    size_t removed_count = 0;
    auto overrides = getComponentOverrides();

    for (const auto& override : overrides) {
        if (ImportanceToolbox::RemoveComponentOverride(override.mapping_id)) {
            removed_count++;
        }
    }

    return removed_count;
}

size_t ImportanceMapper::clearFunctionOverrides() {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    size_t removed_count = 0;
    auto overrides = getFunctionOverrides();

    for (const auto& override : overrides) {
        if (ImportanceToolbox::RemoveFunctionOverride(override.mapping_id)) {
            removed_count++;
        }
    }

    return removed_count;
}

size_t ImportanceMapper::getOverrideCount() const {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    return getComponentOverrides().size() + getFunctionOverrides().size();
}

bool ImportanceMapper::hasOverrides() const {
    std::lock_guard<std::mutex> lock(mapper_mutex_);

    return getOverrideCount() > 0;
}

void ImportanceMapper::swap(ImportanceMapper& other) noexcept {
    std::lock_guard<std::mutex> this_lock(mapper_mutex_);
    std::lock_guard<std::mutex> other_lock(other.mapper_mutex_);

    std::swap(application_name_, other.application_name_);
    std::swap(config_, other.config_);
}

ImportanceMapper ImportanceMapper::clone() const {
    std::lock_guard<std::mutex> lock(mapper_mutex_);
    return ImportanceMapper(application_name_, config_);
}

// =====================================================================================
// NON-MEMBER FUNCTIONS
// =====================================================================================

void swap(ImportanceMapper& a, ImportanceMapper& b) noexcept {
    a.swap(b);
}

std::ostream& operator<<(std::ostream& os, const ImportanceMapper& mapper) {
    os << "ImportanceMapper[";
    if (!mapper.getApplicationName().empty()) {
        os << "application: " << mapper.getApplicationName();
    } else {
        os << "unconfigured";
    }
    os << ", overrides: " << mapper.getOverrideCount() << "]";
    return os;
}

ImportanceMapper CreateApplicationImportanceMapper(const std::string& application_name) {
    return ImportanceMapper(application_name);
}
