#ifndef __ASFM_LOGGER_MESSAGE_TOOLBOX_HPP__
#define __ASFM_LOGGER_MESSAGE_TOOLBOX_HPP__

/**
 * ASFMLogger Message Toolbox
 *
 * Static methods for log message operations following toolbox architecture.
 * Pure functions with no state, maximum testability and reusability.
 */

#include "structs/LogDataStructures.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

// Forward declarations for Windows types
struct _SYSTEMTIME;
struct tm;

class LogMessageToolbox {
private:
    // Only static variables allowed in toolbox
    static std::atomic<uint32_t> global_message_id_counter_;

public:
    // =================================================================================
    // MESSAGE ID GENERATION
    // =================================================================================

    /**
     * @brief Generate a unique message ID
     * @return Unique 32-bit message identifier
     */
    static uint32_t GenerateMessageId();

    /**
     * @brief Reset the message ID counter (primarily for testing)
     */
    static void ResetMessageIdCounter();

    // =================================================================================
    // MESSAGE CREATION AND MANIPULATION
    // =================================================================================

    /**
     * @brief Create a new log message data structure
     * @param type Message type
     * @param message Message content
     * @param component Component name (optional)
     * @param function Function name (optional)
     * @param file Source file (optional)
     * @param line Source line number (optional)
     * @return Configured LogMessageData structure
     */
    static LogMessageData CreateMessage(
        LogMessageType type,
        const std::string& message,
        const std::string& component = "",
        const std::string& function = "",
        const std::string& file = "",
        uint32_t line = 0);

    /**
     * @brief Set message content safely
     * @param data Message data to modify
     * @param message New message content
     * @return true if successful
     */
    static bool SetMessage(LogMessageData& data, const std::string& message);

    /**
     * @brief Set component name safely
     * @param data Message data to modify
     * @param component Component name
     * @return true if successful
     */
    static bool SetComponent(LogMessageData& data, const std::string& component);

    /**
     * @brief Set function name safely
     * @param data Message data to modify
     * @param function Function name
     * @return true if successful
     */
    static bool SetFunction(LogMessageData& data, const std::string& function);

    /**
     * @brief Set source file and line information
     * @param data Message data to modify
     * @param file Source file name
     * @param line Source line number
     * @return true if successful
     */
    static bool SetSourceLocation(LogMessageData& data, const std::string& file, uint32_t line);

    /**
     * @brief Set message type and update severity string
     * @param data Message data to modify
     * @param type New message type
     */
    static void SetType(LogMessageData& data, LogMessageType type);

    // =================================================================================
    // MESSAGE VALIDATION
    // =================================================================================

    /**
     * @brief Validate message data structure
     * @param data Message data to validate
     * @return true if structure appears valid
     */
    static bool ValidateMessage(const LogMessageData& data);

    /**
     * @brief Check if message has valid content
     * @param data Message data to check
     * @return true if message has content
     */
    static bool HasContent(const LogMessageData& data);

    /**
     * @brief Check if message has component information
     * @param data Message data to check
     * @return true if component is set
     */
    static bool HasComponent(const LogMessageData& data);

    // =================================================================================
    // MESSAGE EXTRACTION
    // =================================================================================

    /**
     * @brief Extract message content as string
     * @param data Message data to read
     * @return Message string (null-terminated)
     */
    static std::string ExtractMessage(const LogMessageData& data);

    /**
     * @brief Extract component name as string
     * @param data Message data to read
     * @return Component string (null-terminated)
     */
    static std::string ExtractComponent(const LogMessageData& data);

    /**
     * @brief Extract function name as string
     * @param data Message data to read
     * @return Function string (null-terminated)
     */
    static std::string ExtractFunction(const LogMessageData& data);

    /**
     * @brief Extract source file as string
     * @param data Message data to read
     * @return Source file string (null-terminated)
     */
    static std::string ExtractFile(const LogMessageData& data);

    /**
     * @brief Get message type
     * @param data Message data to read
     * @return Message type
     */
    static LogMessageType GetType(const LogMessageData& data);

    /**
     * @brief Get message ID
     * @param data Message data to read
     * @return Message ID
     */
    static uint32_t GetId(const LogMessageData& data);

    /**
     * @brief Get instance ID
     * @param data Message data to read
     * @return Instance ID
     */
    static uint32_t GetInstanceId(const LogMessageData& data);

    // =================================================================================
    // MESSAGE FORMATTING
    // =================================================================================

    /**
     * @brief Convert message to string representation
     * @param data Message data to format
     * @return Human-readable string representation
     */
    static std::string MessageToString(const LogMessageData& data);

    /**
     * @brief Convert message to JSON format
     * @param data Message data to format
     * @return JSON string representation
     */
    static std::string MessageToJson(const LogMessageData& data);

    /**
     * @brief Convert message to CSV format
     * @param data Message data to format
     * @return CSV string representation
     */
    static std::string MessageToCsv(const LogMessageData& data);

    /**
     * @brief Format message for console output
     * @param data Message data to format
     * @return Console-formatted string
     */
    static std::string FormatForConsole(const LogMessageData& data);

    // =================================================================================
    // MESSAGE FILTERING AND SEARCHING
    // =================================================================================

    /**
     * @brief Filter messages by type
     * @param messages Vector of messages to filter
     * @param type Message type to filter by
     * @return Vector of messages matching the type
     */
    static std::vector<LogMessageData> FilterByType(
        const std::vector<LogMessageData>& messages,
        LogMessageType type);

    /**
     * @brief Filter messages by component
     * @param messages Vector of messages to filter
     * @param component Component name to filter by
     * @return Vector of messages matching the component
     */
    static std::vector<LogMessageData> FilterByComponent(
        const std::vector<LogMessageData>& messages,
        const std::string& component);

    /**
     * @brief Filter messages by importance level
     * @param messages Vector of messages to filter
     * @param min_importance Minimum importance level
     * @return Vector of messages meeting importance criteria
     */
    static std::vector<LogMessageData> FilterByImportance(
        const std::vector<LogMessageData>& messages,
        MessageImportance min_importance);

    /**
     * @brief Search messages by content
     * @param messages Vector of messages to search
     * @param search_term Term to search for
     * @return Vector of messages containing the search term
     */
    static std::vector<LogMessageData> SearchByContent(
        const std::vector<LogMessageData>& messages,
        const std::string& search_term);

    // =================================================================================
    // MESSAGE ANALYSIS
    // =================================================================================

    /**
     * @brief Count messages by type
     * @param messages Vector of messages to analyze
     * @return Vector of pairs (type, count)
     */
    static std::vector<std::pair<LogMessageType, size_t>> CountByType(
        const std::vector<LogMessageData>& messages);

    /**
     * @brief Count messages by component
     * @param messages Vector of messages to analyze
     * @return Vector of pairs (component, count)
     */
    static std::vector<std::pair<std::string, size_t>> CountByComponent(
        const std::vector<LogMessageData>& messages);

    /**
     * @brief Get unique components from messages
     * @param messages Vector of messages to analyze
     * @return Set of unique component names
     */
    static std::vector<std::string> GetUniqueComponents(
        const std::vector<LogMessageData>& messages);

    /**
     * @brief Calculate message rate (messages per second)
     * @param messages Vector of messages to analyze
     * @return Message rate as double
     */
    static double CalculateMessageRate(const std::vector<LogMessageData>& messages);

    // =================================================================================
    // BATCH OPERATIONS
    // =================================================================================

    /**
     * @brief Validate multiple messages
     * @param messages Vector of messages to validate
     * @return Vector of validation results
     */
    static std::vector<bool> ValidateMessages(const std::vector<LogMessageData>& messages);

    /**
     * @brief Sort messages by timestamp
     * @param messages Vector of messages to sort (modified in place)
     */
    static void SortByTimestamp(std::vector<LogMessageData>& messages);

    /**
     * @brief Sort messages by type
     * @param messages Vector of messages to sort (modified in place)
     */
    static void SortByType(std::vector<LogMessageData>& messages);

    /**
     * @brief Remove duplicate messages based on content hash
     * @param messages Vector of messages to deduplicate (modified in place)
     * @return Number of duplicates removed
     */
    static size_t RemoveDuplicates(std::vector<LogMessageData>& messages);

    // =================================================================================
    // MESSAGE HASHING AND COMPARISON
    // =================================================================================

    /**
     * @brief Calculate hash of message content for deduplication
     * @param data Message data to hash
     * @return 32-bit hash of message content
     */
    static uint32_t HashMessageContent(const LogMessageData& data);

    /**
     * @brief Compare two messages for equality
     * @param a First message
     * @param b Second message
     * @return true if messages are identical
     */
    static bool CompareMessages(const LogMessageData& a, const LogMessageData& b);

    /**
     * @brief Check if two messages have the same content
     * @param a First message
     * @param b Second message
     * @return true if message content is identical
     */
    static bool CompareMessageContent(const LogMessageData& a, const LogMessageData& b);

private:
    // Private helper methods
    static uint32_t CalculateContentHash(const char* content);
    static bool AreStringsEqual(const char* a, const char* b);
    static size_t FindStringLength(const char* str, size_t max_length);
};

#endif // __ASFM_LOGGER_MESSAGE_TOOLBOX_HPP__