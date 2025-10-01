#ifndef __ASFM_LOGGER_TIMESTAMP_TOOLBOX_HPP__
#define __ASFM_LOGGER_TIMESTAMP_TOOLBOX_HPP__

/**
 * ASFMLogger Timestamp Toolbox
 *
 * Static methods for timestamp operations following toolbox architecture.
 * Pure functions for high-precision timing operations.
 */

#include "structs/LogDataStructures.hpp"
#include <string>
#include <chrono>
#include <ctime>

// Forward declarations for Windows types
struct _SYSTEMTIME;
struct FILETIME;
struct tm;

class TimestampToolbox {
public:
    // =================================================================================
    // TIMESTAMP CREATION
    // =================================================================================

    /**
     * @brief Create timestamp with current system time
     * @return Current timestamp with high precision
     */
    static LogTimestamp Now();

    /**
     * @brief Create timestamp from Unix time
     * @param unix_seconds Seconds since Unix epoch
     * @return LogTimestamp representation
     */
    static LogTimestamp FromUnixTime(time_t unix_seconds);

    /**
     * @brief Create timestamp from Unix time with microseconds
     * @param unix_seconds Seconds since Unix epoch
     * @param microseconds Microseconds
     * @return LogTimestamp representation
     */
    static LogTimestamp FromUnixTime(time_t unix_seconds, DWORD microseconds);

    /**
     * @brief Create timestamp from Windows SYSTEMTIME
     * @param st Windows SYSTEMTIME structure
     * @return LogTimestamp representation
     */
    static LogTimestamp FromSystemTime(const _SYSTEMTIME& st);

    /**
     * @brief Create timestamp from Windows FILETIME
     * @param ft Windows FILETIME structure
     * @return LogTimestamp representation
     */
    static LogTimestamp FromFileTime(const FILETIME& ft);

    // =================================================================================
    // TIMESTAMP CONVERSION
    // =================================================================================

    /**
     * @brief Convert to Unix timestamp (seconds since epoch)
     * @param timestamp LogTimestamp to convert
     * @return Unix timestamp as time_t
     */
    static time_t ToUnixTime(const LogTimestamp& timestamp);

    /**
     * @brief Convert to total microseconds since epoch
     * @param timestamp LogTimestamp to convert
     * @return Microseconds as uint64_t
     */
    static uint64_t ToMicroseconds(const LogTimestamp& timestamp);

    /**
     * @brief Convert to Windows SYSTEMTIME
     * @param timestamp LogTimestamp to convert
     * @param st Output SYSTEMTIME structure
     */
    static void ToSystemTime(const LogTimestamp& timestamp, _SYSTEMTIME& st);

    /**
     * @brief Convert to Windows FILETIME
     * @param timestamp LogTimestamp to convert
     * @param ft Output FILETIME structure
     */
    static void ToFileTime(const LogTimestamp& timestamp, FILETIME& ft);

    /**
     * @brief Convert to tm structure for standard library compatibility
     * @param timestamp LogTimestamp to convert
     * @param tm_time Output tm structure
     */
    static void ToTmTime(const LogTimestamp& timestamp, tm& tm_time);

    // =================================================================================
    // TIMESTAMP FORMATTING
    // =================================================================================

    /**
     * @brief Convert to string representation
     * @param timestamp Timestamp to format
     * @return Formatted timestamp string (seconds.microseconds)
     */
    static std::string ToString(const LogTimestamp& timestamp);

    /**
     * @brief Format timestamp for human readability
     * @param timestamp Timestamp to format
     * @param include_microseconds Whether to include microsecond precision
     * @return Human-readable timestamp string
     */
    static std::string Format(const LogTimestamp& timestamp, bool include_microseconds = true);

    /**
     * @brief Format timestamp for human readability with custom format
     * @param timestamp Timestamp to format
     * @param format_str Format string (similar to strftime)
     * @return Formatted timestamp string
     */
    static std::string FormatCustom(const LogTimestamp& timestamp, const std::string& format_str);

    /**
     * @brief Format as ISO 8601 timestamp
     * @param timestamp Timestamp to format
     * @return ISO 8601 formatted string
     */
    static std::string ToIso8601(const LogTimestamp& timestamp);

    // =================================================================================
    // TIMESTAMP COMPARISON
    // =================================================================================

    /**
     * @brief Compare two timestamps
     * @param a First timestamp
     * @param b Second timestamp
     * @return -1 if a < b, 0 if a == b, 1 if a > b
     */
    static int Compare(const LogTimestamp& a, const LogTimestamp& b);

    /**
     * @brief Check if timestamp A is before timestamp B
     * @param a First timestamp
     * @param b Second timestamp
     * @return true if a is before b
     */
    static bool IsBefore(const LogTimestamp& a, const LogTimestamp& b);

    /**
     * @brief Check if timestamp A is after timestamp B
     * @param a First timestamp
     * @param b Second timestamp
     * @return true if a is after b
     */
    static bool IsAfter(const LogTimestamp& a, const LogTimestamp& b);

    /**
     * @brief Check if timestamps are equal
     * @param a First timestamp
     * @param b Second timestamp
     * @return true if timestamps are equal
     */
    static bool AreEqual(const LogTimestamp& a, const LogTimestamp& b);

    /**
     * @brief Calculate difference between two timestamps
     * @param earlier Earlier timestamp
     * @param later Later timestamp
     * @return Difference in microseconds
     */
    static int64_t DifferenceMicroseconds(const LogTimestamp& earlier, const LogTimestamp& later);

    /**
     * @brief Calculate difference between two timestamps
     * @param earlier Earlier timestamp
     * @param later Later timestamp
     * @return Difference in milliseconds
     */
    static int64_t DifferenceMilliseconds(const LogTimestamp& earlier, const LogTimestamp& later);

    // =================================================================================
    // TIMESTAMP ARITHMETIC
    // =================================================================================

    /**
     * @brief Add microseconds to timestamp
     * @param timestamp Base timestamp
     * @param microseconds Microseconds to add
     * @return New timestamp with added time
     */
    static LogTimestamp AddMicroseconds(const LogTimestamp& timestamp, uint64_t microseconds);

    /**
     * @brief Add milliseconds to timestamp
     * @param timestamp Base timestamp
     * @param milliseconds Milliseconds to add
     * @return New timestamp with added time
     */
    static LogTimestamp AddMilliseconds(const LogTimestamp& timestamp, uint64_t milliseconds);

    /**
     * @brief Add seconds to timestamp
     * @param timestamp Base timestamp
     * @param seconds Seconds to add
     * @return New timestamp with added time
     */
    static LogTimestamp AddSeconds(const LogTimestamp& timestamp, uint64_t seconds);

    /**
     * @brief Subtract microseconds from timestamp
     * @param timestamp Base timestamp
     * @param microseconds Microseconds to subtract
     * @return New timestamp with subtracted time
     */
    static LogTimestamp SubtractMicroseconds(const LogTimestamp& timestamp, uint64_t microseconds);

    // =================================================================================
    // TIMESTAMP VALIDATION
    // =================================================================================

    /**
     * @brief Validate timestamp structure
     * @param timestamp Timestamp to validate
     * @return true if timestamp appears valid
     */
    static bool ValidateTimestamp(const LogTimestamp& timestamp);

    /**
     * @brief Check if timestamp is within valid range
     * @param timestamp Timestamp to check
     * @param earliest_acceptable Earliest acceptable timestamp
     * @param latest_acceptable Latest acceptable timestamp
     * @return true if timestamp is within range
     */
    static bool IsWithinRange(const LogTimestamp& timestamp,
                             const LogTimestamp& earliest_acceptable,
                             const LogTimestamp& latest_acceptable);

    /**
     * @brief Check if timestamp represents a future time
     * @param timestamp Timestamp to check
     * @return true if timestamp is in the future
     */
    static bool IsFuture(const LogTimestamp& timestamp);

    /**
     * @brief Check if timestamp represents a past time
     * @param timestamp Timestamp to check
     * @return true if timestamp is in the past
     */
    static bool IsPast(const LogTimestamp& timestamp);

    // =================================================================================
    // PERFORMANCE TIMING
    // =================================================================================

    /**
     * @brief Get high-resolution timer frequency
     * @return Timer frequency in Hz
     */
    static uint64_t GetTimerFrequency();

    /**
     * @brief Get high-resolution timer value
     * @return Current timer value
     */
    static uint64_t GetTimerValue();

    /**
     * @brief Calculate elapsed time between timer values
     * @param start_timer Start timer value
     * @param end_timer End timer value
     * @return Elapsed time in microseconds
     */
    static uint64_t CalculateElapsedMicroseconds(uint64_t start_timer, uint64_t end_timer);

    /**
     * @brief Create timestamp from high-resolution timer
     * @param timer_value High-resolution timer value
     * @return LogTimestamp representation
     */
    static LogTimestamp FromTimerValue(uint64_t timer_value);

private:
    // Private helper methods
    static uint64_t FileTimeToUnixMicroseconds(const FILETIME& ft);
    static FILETIME UnixMicrosecondsToFileTime(uint64_t microseconds);
    static void UnixTimeToTm(time_t unix_seconds, tm& tm_time);
    static time_t TmToUnixTime(const tm& tm_time);
    static bool IsLeapYear(int year);
    static int GetDaysInMonth(int month, int year);
};

#endif // __ASFM_LOGGER_TIMESTAMP_TOOLBOX_HPP__