#ifndef __ASFM_LOGGER_IMPORTANCE_CONFIG_HPP__
#define __ASFM_LOGGER_IMPORTANCE_CONFIG_HPP__

/**
 * ASFMLogger Importance Framework POD Structures
 *
 * Pure data structures for importance configuration and mapping following toolbox architecture.
 * No methods, pure data for maximum compatibility and testability.
 */

#include "structs/LogDataStructures.hpp"
#include <cstdint>

// =====================================================================================
// IMPORTANCE MAPPING DATA STRUCTURES
// =====================================================================================

/**
 * @brief POD structure for type-based importance mapping
 * Maps LogMessageType to MessageImportance with optional overrides
 */
struct ImportanceMapping {
    LogMessageType message_type;           ///< Message type being mapped
    MessageImportance default_importance;  ///< Default importance for this type
    MessageImportance override_importance; ///< Override importance (if used)
    bool use_override;                     ///< Whether override is active
    char reason[256];                      ///< Reason for override (optional)
    DWORD last_updated;                    ///< When mapping was last updated
    char reserved[128];                    ///< Future extensibility
};

/**
 * @brief POD structure for component-based importance override
 * Allows different importance levels for different application components
 */
struct ComponentImportanceOverride {
    uint32_t mapping_id;                   ///< Unique mapping identifier
    char component_pattern[256];           ///< Component name pattern (supports wildcards)
    MessageImportance importance;          ///< Importance level for this component
    bool use_regex;                        ///< Whether pattern is regex
    DWORD created_time;                    ///< When override was created
    char created_by[128];                  ///< Who created this override
    char reason[256];                      ///< Reason for override
    char reserved[128];                    ///< Future extensibility
};

/**
 * @brief POD structure for function-based importance override
 * Allows different importance levels for different functions/methods
 */
struct FunctionImportanceOverride {
    uint32_t mapping_id;                   ///< Unique mapping identifier
    char function_pattern[256];            ///< Function name pattern (supports wildcards)
    MessageImportance importance;          ///< Importance level for this function
    bool use_regex;                        ///< Whether pattern is regex
    DWORD created_time;                    ///< When override was created
    char created_by[128];                  ///< Who created this override
    char reason[256];                      ///< Reason for override
    char reserved[128];                    ///< Future extensibility
};

/**
 * @brief POD structure for application-specific importance configuration
 * Contains all importance settings for a specific application
 */
struct ApplicationImportanceConfig {
    uint32_t config_id;                    ///< Unique configuration identifier
    char application_name[256];            ///< Application this config applies to
    ImportanceMapping type_mappings[6];    ///< Mapping for each message type (TRACE->CRITICAL)
    ComponentImportanceOverride component_overrides[32];  ///< Component-specific overrides
    FunctionImportanceOverride function_overrides[32];   ///< Function-specific overrides
    size_t component_override_count;       ///< Number of active component overrides
    size_t function_override_count;        ///< Number of active function overrides
    DWORD last_modified;                   ///< When config was last modified
    char modified_by[128];                 ///< Who last modified this config
    bool is_active;                        ///< Whether this config is active
    char reserved[256];                    ///< Future extensibility
};

/**
 * @brief POD structure for importance resolution context
 * Contains all information needed to resolve message importance
 */
struct ImportanceResolutionContext {
    LogMessageType message_type;           ///< Type of the message
    char component[128];                   ///< Component that generated the message
    char function[128];                    ///< Function that generated the message
    char application_name[256];            ///< Application generating the message
    DWORD system_load;                     ///< Current system load (0-100)
    DWORD error_rate;                      ///< Current error rate (errors per second)
    DWORD message_rate;                    ///< Current message rate (messages per second)
    bool is_emergency_mode;                ///< Whether system is in emergency mode
    char reserved[256];                    ///< Future extensibility
};

/**
 * @brief POD structure for importance resolution result
 * Contains the final importance decision and reasoning
 */
struct ImportanceResolutionResult {
    MessageImportance final_importance;    ///< Final resolved importance
    bool was_overridden;                   ///< Whether override was applied
    char applied_override_type[32];        ///< Type of override applied ("TYPE", "COMPONENT", "FUNCTION")
    char override_reason[256];             ///< Reason for override
    uint32_t resolution_time_microseconds; ///< Time taken to resolve importance
    DWORD resolved_at;                     ///< When importance was resolved
    char reserved[128];                    ///< Future extensibility
};

#endif // __ASFM_LOGGER_IMPORTANCE_CONFIG_HPP__