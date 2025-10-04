/**
 * ASFMLogger Ultra-Specialized Core Component Testing
 * TASK 1.03A: ImportanceMapper Deep-Dive Testing
 * Purpose: Exhaustive validation of importance resolution hierarchy and overrides
 * Business Value: Logic cornerstone for contextual persistence - zero mapping risk (⭐⭐⭐⭐⭐)
 */

#ifndef TEST_CORE_IMPORTANCE_MAPPER_HPP
#define TEST_CORE_IMPORTANCE_MAPPER_HPP

// Test includes must come first for GTest
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <algorithm>
#include <numeric>
#include <random>
#include <sstream>
#include <filesystem>
#include <fstream>

// Core logger includes
#include "../src/stateful/ImportanceMapper.hpp"

// Importance mapping testing components
namespace ImportanceTesting {

    /**
     * @brief Importance levels for testing
     */
    enum class Importance {
        TRACE = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        CRITICAL = 5,
        MAX_IMPORTANCE
    };

    /**
     * @brief Test data structure for importance resolution scenarios
     */
    struct ImportanceTestScenario {
        std::string function_name;
        std::string component_path;
        std::string requested_level;
        Importance expected_importance;
        std::string description;

        ImportanceTestScenario(const std::string& func, const std::string& comp,
                             const std::string& level, Importance expected, const std::string& desc)
            : function_name(func), component_path(comp), requested_level(level),
              expected_importance(expected), description(desc) {}
    };

    /**
     * @brief Enhanced ImportanceMapper with testing hooks and diagnostics
     */
    class TestableImportanceMapper {
    public:
        TestableImportanceMapper() : next_rule_id_(1) {}

        // Core mapping functionality
        Importance resolveImportance(const std::string& function, const std::string& component,
                                   const std::string& level) const {
            // First check for exact component matches
            auto component_it = component_overrides_.find(component);
            if (component_it != component_overrides_.end()) {
                return component_it->second;
            }

            // Check hierarchical component matching (most specific first)
            Importance hierarchical_result = findHierarchicalOverride(component);
            if (hierarchical_result != Importance::MAX_IMPORTANCE) {
                return hierarchical_result;
            }

            // Check function + component combination
            auto func_comp_key = function + "@" + component;
            auto func_comp_it = function_component_overrides_.find(func_comp_key);
            if (func_comp_it != function_component_overrides_.end()) {
                return func_comp_it->second;
            }

            // Check wildcard patterns (*)
            Importance wildcard_result = evaluateWildcardPatterns(function, component);
            if (wildcard_result != Importance::MAX_IMPORTANCE) {
                return wildcard_result;
            }

            // Default mapping from string level
            return stringToImportance(level);
        }

        void addComponentOverride(const std::string& component_path, Importance importance) {
            component_overrides_[component_path] = importance;
            rule_history_.emplace_back(next_rule_id_++, RuleType::COMPONENT_OVERRIDE,
                                     component_path, importance, "Component override added");
        }

        void addFunctionComponentOverride(const std::string& function, const std::string& component,
                                        Importance importance) {
            auto key = function + "@" + component;
            function_component_overrides_[key] = importance;
            rule_history_.emplace_back(next_rule_id_++, RuleType::FUNCTION_COMPONENT_OVERRIDE,
                                     key, importance, "Function-component override added");
        }

        void addWildcardPattern(const std::string& pattern, Importance importance) {
            wildcard_patterns_[pattern] = importance;
            rule_history_.emplace_back(next_rule_id_++, RuleType::WILDCARD_PATTERN,
                                     pattern, importance, "Wildcard pattern added");
        }

        void clearAllOverrides() {
            component_overrides_.clear();
            function_component_overrides_.clear();
            wildcard_patterns_.clear();
            rule_history_.clear();
            next_rule_id_ = 1;
        }

        // Advanced testing utilities
        std::vector<std::string> getAllComponentOverrides() const {
            std::vector<std::string> result;
            for (const auto& pair : component_overrides_) {
                result.push_back(pair.first + "=" + importanceToString(pair.second));
            }
            return result;
        }

        std::vector<std::string> getAllFunctionComponentOverrides() const {
            std::vector<std::string> result;
            for (const auto& pair : function_component_overrides_) {
                result.push_back(pair.first + "=" + importanceToString(pair.second));
            }
            return result;
        }

        std::vector<std::string> getAllWildcardPatterns() const {
            std::vector<std::string> result;
            for (const auto& pair : wildcard_patterns_) {
                result.push_back(pair.first + "=" + importanceToString(pair.second));
            }
            return result;
        }

        struct RuleHistory {
            size_t rule_id;
            enum class RuleType { COMPONENT_OVERRIDE, FUNCTION_COMPONENT_OVERRIDE, WILDCARD_PATTERN } type;
            std::string rule_pattern;
            Importance importance;
            std::string description;

            RuleHistory(size_t id, RuleType t, const std::string& pattern,
                       Importance imp, const std::string& desc)
                : rule_id(id), type(t), rule_pattern(pattern), importance(imp), description(desc) {}
        };

        std::vector<RuleHistory> getRuleHistory() const {
            return rule_history_;
        }

        size_t getRuleCount(RuleHistory::RuleType type) const {
            return std::count_if(rule_history_.begin(), rule_history_.end(),
                               [type](const RuleHistory& rule) { return rule.type == type; });
        }

        // Performance analysis
        struct ResolutionStats {
            size_t total_resolutions;
            size_t component_matches;
            size_t hierarchical_matches;
            size_t function_component_matches;
            size_t wildcard_matches;
            size_t default_fallbacks;
            std::chrono::microseconds total_resolution_time;

            ResolutionStats() : total_resolutions(0), component_matches(0), hierarchical_matches(0),
                              function_component_matches(0), wildcard_matches(0), default_fallbacks(0),
                              total_resolution_time(0) {}
        };

        ResolutionStats getResolutionStats() const {
            return resolution_stats_;
        }

    private:
        mutable ResolutionStats resolution_stats_;
        std::unordered_map<std::string, Importance> component_overrides_;
        std::unordered_map<std::string, Importance> function_component_overrides_;
        std::unordered_map<std::string, Importance> wildcard_patterns_;
        std::vector<RuleHistory> rule_history_;
        size_t next_rule_id_;

        Importance findHierarchicalOverride(const std::string& component_path) const {
            // Split component path and check from most specific to least specific
            std::vector<std::string> path_parts;
            std::istringstream iss(component_path);
            std::string part;
            while (std::getline(iss, part, '.')) {
                path_parts.push_back(part);
            }

            // Build hierarchical paths from most specific to least specific
            for (size_t i = path_parts.size(); i > 0; --i) {
                std::string current_path;
                for (size_t j = 0; j < i; ++j) {
                    if (j > 0) current_path += ".";
                    current_path += path_parts[j];
                }

                auto it = component_overrides_.find(current_path);
                if (it != component_overrides_.end()) {
                    resolution_stats_.hierarchical_matches++;
                    return it->second;
                }
            }

            return Importance::MAX_IMPORTANCE;
        }

        Importance evaluateWildcardPatterns(const std::string& function, const std::string& component) const {
            // Check wildcard patterns - simple implementation for testing
            // Real implementation would use regex or pattern matching
            for (const auto& pattern_pair : wildcard_patterns_) {
                const std::string& pattern = pattern_pair.first;

                // Simple wildcard matching (real implementation would be more sophisticated)
                if (pattern == "*" && !component.empty()) {
                    resolution_stats_.wildcard_matches++;
                    return pattern_pair.second;
                }

                if (pattern == "*.*" && component.find('.') != std::string::npos) {
                    resolution_stats_.wildcard_matches++;
                    return pattern_pair.second;
                }

                // Function-specific patterns
                if (pattern.find(function) != std::string::npos ||
                    pattern.find("*" + component) != std::string::npos) {
                    resolution_stats_.wildcard_matches++;
                    return pattern_pair.second;
                }
            }

            return Importance::MAX_IMPORTANCE;
        }

        Importance stringToImportance(const std::string& level) const {
            static const std::unordered_map<std::string, Importance> string_to_importance = {
                {"TRACE", Importance::TRACE},
                {"DEBUG", Importance::DEBUG},
                {"INFO", Importance::INFO},
                {"WARN", Importance::WARN},
                {"WARNING", Importance::WARN},
                {"ERROR", Importance::ERROR},
                {"CRITICAL", Importance::CRITICAL},
                {"FATAL", Importance::CRITICAL}
            };

            auto it = string_to_importance.find(level);
            return (it != string_to_importance.end()) ? it->second : Importance::INFO;
        }

        std::string importanceToString(Importance imp) const {
            switch (imp) {
                case Importance::TRACE: return "TRACE";
                case Importance::DEBUG: return "DEBUG";
                case Importance::INFO: return "INFO";
                case Importance::WARN: return "WARN";
                case Importance::ERROR: return "ERROR";
                case Importance::CRITICAL: return "CRITICAL";
                default: return "UNKNOWN";
            }
        }
    };

    /**
     * @brief Comprehensive test scenario generator
     */
    class ImportanceMappingTestGenerator {
    public:
        static std::vector<ImportanceTestScenario> generateHierarchicalScenarios() {
            return {
                // Basic hierarchical matching
                {"ProcessData", "MyApp.Database", "INFO", Importance::DEBUG, "Component exact match"},
                {"SaveRecord", "MyApp.Database.Connection", "DEBUG", Importance::DEBUG, "Hierarchical inheritance"},
                {"ExecuteQuery", "MyApp.Database.Connection.Pool", "INFO", Importance::DEBUG, "Deep hierarchical inheritance"},

                // Sibling components (should not inherit)
                {"ValidateInput", "MyApp.Validation", "WARN", Importance::INFO, "Sibling no inheritance"},

                // Root level overrides
                {"HandleError", "System", "ERROR", Importance::CRITICAL, "Root level override"},
                {"MonitorHealth", "System.Health", "INFO", Importance::CRITICAL, "Inherited root override"}
            };
        }

        static std::vector<ImportanceTestScenario> generateFunctionComponentScenarios() {
            return {
                // Function + component specific overrides
                {"SaveData", "MyApp.Database", "INFO", Importance::CRITICAL, "Function-component exact match"},
                {"ProcessData", "MyApp.Database", "DEBUG", Importance::INFO, "Different function same component"},

                // Function specificity (exact matches should override patterns)
                {"ExecuteQuery", "MyApp.Database.Connection", "INFO", Importance::WARN, "Function-specific override"},

                // Multiple functions with same component
                {"ValidateUser", "MyApp.Security", "DEBUG", Importance::ERROR, "Security function critical"},
                {"LogActivity", "MyApp.Security", "INFO", Importance::WARN, "Security logging medium priority"}
            };
        }

        static std::vector<ImportanceTestScenario> generateWildcardScenarios() {
            return {
                // Wildcard component patterns
                {"ProcessData", "Any.Component", "INFO", Importance::TRACE, "Global component wildcard"},
                {"HandleRequest", "API.Endpoint", "DEBUG", Importance::DEBUG, "API wildcard pattern"},

                // Function-based wildcards
                {"ExceptionHandler", "MyApp.Exceptions", "ERROR", Importance::CRITICAL, "Exception function wildcard"},

                // Complex patterns (would be more sophisticated in real implementation)
                {"AsyncOperation", "Background.Task", "INFO", Importance::WARN, "Async operation pattern"}
            };
        }

        static std::vector<ImportanceTestScenario> generatePrecedenceTestScenarios() {
            return {
                // Override precedence testing
                {"SaveData", "MyApp.Database", "DEBUG", Importance::CRITICAL, "Function-component should override component"},
                {"QuickSave", "MyApp.Database", "INFO", Importance::WARN, "Component override (no function match)"},

                // Specificity hierarchy
                {"ExecuteQuery", "MyApp.Database.Connection", "DEBUG", Importance::ERROR,
                 "Function-component most specific - should win"},
                {"FetchData", "MyApp.Database.Connection", "INFO", Importance::CRITICAL,
                 "Hierarchical component match"},
                {"FastQuery", "MyApp.Database.Connection", "WARN", Importance::INFO,
                 "Default level (no matches)"}
            };
        }
    };

} // namespace ImportanceTesting

#endif // TEST_CORE_IMPORTANCE_MAPPER_HPP
