#ifndef __ASFM_LOGGER_CORE_HPP__
#define __ASFM_LOGGER_CORE_HPP__

/**
 * ASFMLogger Core Data Structures and Types
 *
 * This header defines the foundational data structures for the enhanced ASFMLogger,
 * including message types, importance levels, timestamps, and core data structures
 * for shared memory compatibility and cross-platform support.
 */

#include <string>
#include <chrono>
#include <cstdint>
#include <Windows.h>
#include "asfmlogger/structs/LogDataStructures.hpp"

// Forward declarations for Windows types
struct _SYSTEMTIME;
struct tm;

/**
 * @brief Convert LogMessageType to string representation
 * @param type The message type to convert
 * @return String representation of the message type
 */
inline std::string LogMessageTypeToString(LogMessageType type) {
    switch (type) {
        case LogMessageType::TRACE: return "TRACE";
        case LogMessageType::DEBUG: return "DEBUG";
        case LogMessageType::INFO: return "INFO";
        case LogMessageType::WARN: return "WARN";
        case LogMessageType::ERR: return "ERROR";
        case LogMessageType::CRITICAL_LOG: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Convert MessageImportance to string representation
 * @param importance The importance level to convert
 * @return String representation of the importance level
 */
inline std::string MessageImportanceToString(MessageImportance importance) {
    switch (importance) {
        case MessageImportance::LOW: return "LOW";
        case MessageImportance::MEDIUM: return "MEDIUM";
        case MessageImportance::HIGH: return "HIGH";
        case MessageImportance::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Convert string to LogMessageType
 * @param str String representation of message type
 * @return Corresponding LogMessageType or TRACE as default
 */
inline LogMessageType StringToLogMessageType(const std::string& str) {
    if (str == "TRACE") return LogMessageType::TRACE;
    if (str == "DEBUG") return LogMessageType::DEBUG;
    if (str == "INFO") return LogMessageType::INFO;
    if (str == "WARN") return LogMessageType::WARN;
    if (str == "ERROR" || str == "ERR") return LogMessageType::ERR;
    if (str == "CRITICAL") return LogMessageType::CRITICAL_LOG;
    return LogMessageType::TRACE; // Default fallback
}

/**
 * @brief Convert string to MessageImportance
 * @param str String representation of importance level
 * @return Corresponding MessageImportance or LOW as default
 */
inline MessageImportance StringToMessageImportance(const std::string& str) {
    if (str == "LOW") return MessageImportance::LOW;
    if (str == "MEDIUM") return MessageImportance::MEDIUM;
    if (str == "HIGH") return MessageImportance::HIGH;
    if (str == "CRITICAL") return MessageImportance::CRITICAL;
    return MessageImportance::LOW; // Default fallback
}

// =====================================================================================
// UTILITY FUNCTIONS
// =====================================================================================

/**
 * @brief Generate a unique message ID across the system
 * @return Unique 32-bit message identifier
 */
uint32_t GenerateUniqueMessageId();

/**
 * @brief Get current process ID (Windows)
 * @return Current process ID
 */
DWORD GetCurrentProcessIdImpl();

/**
 * @brief Get current thread ID (Windows)
 * @return Current thread ID
 */
DWORD GetCurrentThreadIdImpl();

/**
 * @brief Safe string copy with length checking
 * @param dest Destination buffer
 * @param src Source string
 * @param dest_size Size of destination buffer
 * @return Number of characters copied
 */
size_t SafeStringCopy(char* dest, const std::string& src, size_t dest_size);

/**
 * @brief Safe string copy with length checking and padding
 * @param dest Destination buffer
 * @param src Source string
 * @param dest_size Size of destination buffer
 * @param padding_char Character to use for padding
 * @return Number of characters copied
 */
size_t SafeStringCopy(char* dest, const std::string& src, size_t dest_size, char padding_char);

/**
 * @brief Convert null-terminated char array to string
 * @param char_array Null-terminated character array
 * @return String representation
 */
std::string CharArrayToString(const char* char_array);

/**
 * @brief Convert char array with length to string
 * @param char_array Character array
 * @param length Length of the array
 * @return String representation
 */
std::string CharArrayToString(const char* char_array, size_t length);

#endif // __ASFM_LOGGER_CORE_HPP__