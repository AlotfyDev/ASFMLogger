/**
 * ASFMLogger Timestamp Toolbox Implementation
 *
 * Static methods for timestamp operations following toolbox architecture.
 * Pure functions for high-precision timing operations.
 */

#include "TimestampToolbox.hpp"
#include "../ASFMLoggerCore.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <Windows.h>
#include <time.h>
#include <chrono>

// =====================================================================================
// TIMESTAMP CREATION
// =====================================================================================

LogTimestamp TimestampToolbox::Now() {
    LogTimestamp timestamp;

    // Get current system time with high precision
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();

    // Convert to seconds and microseconds
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration);
    timestamp.seconds = static_cast<DWORD>(microseconds.count() / 1000000);
    timestamp.microseconds = static_cast<DWORD>(microseconds.count() % 1000000);
    timestamp.milliseconds = static_cast<DWORD>(microseconds.count() / 1000);

    return timestamp;
}

LogTimestamp TimestampToolbox::FromUnixTime(time_t unix_seconds) {
    LogTimestamp timestamp;
    timestamp.seconds = static_cast<DWORD>(unix_seconds);
    timestamp.microseconds = 0;
    timestamp.milliseconds = static_cast<DWORD>(unix_seconds * 1000);
    return timestamp;
}

LogTimestamp TimestampToolbox::FromUnixTime(time_t unix_seconds, DWORD microseconds) {
    LogTimestamp timestamp;
    timestamp.seconds = static_cast<DWORD>(unix_seconds);
    timestamp.microseconds = microseconds;
    timestamp.milliseconds = static_cast<DWORD>(unix_seconds * 1000 + microseconds / 1000);
    return timestamp;
}

LogTimestamp TimestampToolbox::FromSystemTime(const _SYSTEMTIME& st) {
    LogTimestamp timestamp;

    // Convert SYSTEMTIME to time_t
    tm tm_time = {};
    tm_time.tm_year = st.wYear - 1900;
    tm_time.tm_mon = st.wMonth - 1;
    tm_time.tm_mday = st.wDay;
    tm_time.tm_hour = st.wHour;
    tm_time.tm_min = st.wMinute;
    tm_time.tm_sec = st.wSecond;
    tm_time.tm_isdst = -1; // Let CRT determine DST

    time_t unix_seconds = mktime(&tm_time);
    timestamp.seconds = static_cast<DWORD>(unix_seconds);
    timestamp.microseconds = static_cast<DWORD>(st.wMilliseconds * 1000);
    timestamp.milliseconds = st.wMilliseconds;

    return timestamp;
}

LogTimestamp TimestampToolbox::FromFileTime(const FILETIME& ft) {
    return FromTimerValue(FileTimeToUnixMicroseconds(ft));
}

// =====================================================================================
// TIMESTAMP CONVERSION
// =====================================================================================

time_t TimestampToolbox::ToUnixTime(const LogTimestamp& timestamp) {
    return static_cast<time_t>(timestamp.seconds);
}

uint64_t TimestampToolbox::ToMicroseconds(const LogTimestamp& timestamp) {
    return (static_cast<uint64_t>(timestamp.seconds) * 1000000ULL) + timestamp.microseconds;
}

void TimestampToolbox::ToSystemTime(const LogTimestamp& timestamp, _SYSTEMTIME& st) {
    // Convert Unix timestamp to SYSTEMTIME
    time_t unix_seconds = ToUnixTime(timestamp);
    tm tm_time;

    UnixTimeToTm(unix_seconds, tm_time);

    st.wYear = static_cast<WORD>(tm_time.tm_year + 1900);
    st.wMonth = static_cast<WORD>(tm_time.tm_mon + 1);
    st.wDayOfWeek = static_cast<WORD>(tm_time.tm_wday);
    st.wDay = static_cast<WORD>(tm_time.tm_mday);
    st.wHour = static_cast<WORD>(tm_time.tm_hour);
    st.wMinute = static_cast<WORD>(tm_time.tm_min);
    st.wSecond = static_cast<WORD>(tm_time.tm_sec);
    st.wMilliseconds = static_cast<WORD>(timestamp.microseconds / 1000);
}

void TimestampToolbox::ToFileTime(const LogTimestamp& timestamp, FILETIME& ft) {
    uint64_t microseconds = ToMicroseconds(timestamp);
    ft = UnixMicrosecondsToFileTime(microseconds);
}

void TimestampToolbox::ToTmTime(const LogTimestamp& timestamp, tm& tm_time) {
    time_t unix_seconds = ToUnixTime(timestamp);
    UnixTimeToTm(unix_seconds, tm_time);
}

// =====================================================================================
// TIMESTAMP FORMATTING
// =====================================================================================

std::string TimestampToolbox::ToString(const LogTimestamp& timestamp) {
    std::ostringstream oss;
    oss << timestamp.seconds << "." << std::setfill('0') << std::setw(6) << timestamp.microseconds;
    return oss.str();
}

std::string TimestampToolbox::Format(const LogTimestamp& timestamp, bool include_microseconds) {
    time_t unix_seconds = ToUnixTime(timestamp);
    tm tm_time;

    UnixTimeToTm(unix_seconds, tm_time);

    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(4) << (tm_time.tm_year + 1900) << "-"
        << std::setw(2) << (tm_time.tm_mon + 1) << "-"
        << std::setw(2) << tm_time.tm_mday << " "
        << std::setw(2) << tm_time.tm_hour << ":"
        << std::setw(2) << tm_time.tm_min << ":"
        << std::setw(2) << tm_time.tm_sec;

    if (include_microseconds) {
        oss << "." << std::setw(6) << timestamp.microseconds;
    }

    return oss.str();
}

std::string TimestampToolbox::FormatCustom(const LogTimestamp& timestamp, const std::string& format_str) {
    time_t unix_seconds = ToUnixTime(timestamp);
    tm tm_time;

    UnixTimeToTm(unix_seconds, tm_time);

    // Simple custom format implementation (could be enhanced)
    std::string result = format_str;

    // Replace format specifiers
    size_t pos = 0;
    while ((pos = result.find("%Y", pos)) != std::string::npos) {
        std::string year = std::to_string(tm_time.tm_year + 1900);
        result.replace(pos, 2, year);
        pos += year.length();
    }

    pos = 0;
    while ((pos = result.find("%m", pos)) != std::string::npos) {
        std::string month = std::to_string(tm_time.tm_mon + 1);
        if (month.length() == 1) month = "0" + month;
        result.replace(pos, 2, month);
        pos += 2;
    }

    pos = 0;
    while ((pos = result.find("%d", pos)) != std::string::npos) {
        std::string day = std::to_string(tm_time.tm_mday);
        if (day.length() == 1) day = "0" + day;
        result.replace(pos, 2, day);
        pos += 2;
    }

    // Add more format specifiers as needed

    return result;
}

std::string TimestampToolbox::ToIso8601(const LogTimestamp& timestamp) {
    std::ostringstream oss;
    oss << Format(timestamp, true) << "Z";
    return oss.str();
}

// =====================================================================================
// TIMESTAMP COMPARISON
// =====================================================================================

int TimestampToolbox::Compare(const LogTimestamp& a, const LogTimestamp& b) {
    if (a.seconds < b.seconds) return -1;
    if (a.seconds > b.seconds) return 1;
    if (a.microseconds < b.microseconds) return -1;
    if (a.microseconds > b.microseconds) return 1;
    return 0;
}

bool TimestampToolbox::IsBefore(const LogTimestamp& a, const LogTimestamp& b) {
    return Compare(a, b) < 0;
}

bool TimestampToolbox::IsAfter(const LogTimestamp& a, const LogTimestamp& b) {
    return Compare(a, b) > 0;
}

bool TimestampToolbox::AreEqual(const LogTimestamp& a, const LogTimestamp& b) {
    return Compare(a, b) == 0;
}

int64_t TimestampToolbox::DifferenceMicroseconds(const LogTimestamp& earlier, const LogTimestamp& later) {
    uint64_t earlier_us = ToMicroseconds(earlier);
    uint64_t later_us = ToMicroseconds(later);
    return static_cast<int64_t>(later_us - earlier_us);
}

int64_t TimestampToolbox::DifferenceMilliseconds(const LogTimestamp& earlier, const LogTimestamp& later) {
    return DifferenceMicroseconds(earlier, later) / 1000;
}

// =====================================================================================
// TIMESTAMP ARITHMETIC
// =====================================================================================

LogTimestamp TimestampToolbox::AddMicroseconds(const LogTimestamp& timestamp, uint64_t microseconds) {
    uint64_t total_microseconds = ToMicroseconds(timestamp) + microseconds;
    return FromTimerValue(total_microseconds);
}

LogTimestamp TimestampToolbox::AddMilliseconds(const LogTimestamp& timestamp, uint64_t milliseconds) {
    return AddMicroseconds(timestamp, milliseconds * 1000);
}

LogTimestamp TimestampToolbox::AddSeconds(const LogTimestamp& timestamp, uint64_t seconds) {
    LogTimestamp result = timestamp;
    result.seconds += static_cast<DWORD>(seconds);

    // Handle carry-over from seconds to minutes, hours, etc. if needed
    // For simplicity, we'll just add the seconds
    DWORD additional_milliseconds = static_cast<DWORD>(seconds * 1000);
    result.milliseconds += additional_milliseconds;

    return result;
}

LogTimestamp TimestampToolbox::SubtractMicroseconds(const LogTimestamp& timestamp, uint64_t microseconds) {
    uint64_t total_microseconds = ToMicroseconds(timestamp);
    if (total_microseconds < microseconds) {
        // Handle underflow - return zero timestamp
        LogTimestamp zero;
        zero.seconds = 0;
        zero.microseconds = 0;
        zero.milliseconds = 0;
        return zero;
    }

    return FromTimerValue(total_microseconds - microseconds);
}

// =====================================================================================
// TIMESTAMP VALIDATION
// =====================================================================================

bool TimestampToolbox::ValidateTimestamp(const LogTimestamp& timestamp) {
    // Basic validation - check for reasonable ranges
    return timestamp.seconds >= 0 &&
           timestamp.microseconds < 1000000 &&
           timestamp.milliseconds >= 0;
}

bool TimestampToolbox::IsWithinRange(const LogTimestamp& timestamp,
                                   const LogTimestamp& earliest_acceptable,
                                   const LogTimestamp& latest_acceptable) {
    return !IsBefore(timestamp, earliest_acceptable) &&
           !IsAfter(timestamp, latest_acceptable);
}

bool TimestampToolbox::IsFuture(const LogTimestamp& timestamp) {
    LogTimestamp now = Now();
    return IsAfter(timestamp, now);
}

bool TimestampToolbox::IsPast(const LogTimestamp& timestamp) {
    LogTimestamp now = Now();
    return IsBefore(timestamp, now);
}

// =====================================================================================
// PERFORMANCE TIMING
// =====================================================================================

uint64_t TimestampToolbox::GetTimerFrequency() {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    return static_cast<uint64_t>(frequency.QuadPart);
}

uint64_t TimestampToolbox::GetTimerValue() {
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return static_cast<uint64_t>(counter.QuadPart);
}

uint64_t TimestampToolbox::CalculateElapsedMicroseconds(uint64_t start_timer, uint64_t end_timer) {
    uint64_t frequency = GetTimerFrequency();
    uint64_t elapsed = end_timer - start_timer;
    return (elapsed * 1000000ULL) / frequency;
}

LogTimestamp TimestampToolbox::FromTimerValue(uint64_t timer_value) {
    // Convert high-resolution timer value to LogTimestamp
    uint64_t frequency = GetTimerFrequency();
    uint64_t microseconds = (timer_value * 1000000ULL) / frequency;

    LogTimestamp timestamp;
    timestamp.seconds = static_cast<DWORD>(microseconds / 1000000);
    timestamp.microseconds = static_cast<DWORD>(microseconds % 1000000);
    timestamp.milliseconds = static_cast<DWORD>(microseconds / 1000);

    return timestamp;
}

// =====================================================================================
// PRIVATE HELPER METHODS
// =====================================================================================

uint64_t TimestampToolbox::FileTimeToUnixMicroseconds(const FILETIME& ft) {
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;

    // Convert from Windows FILETIME (100-nanosecond intervals since Jan 1, 1601)
    // to Unix microseconds (microseconds since Jan 1, 1970)
    const uint64_t WINDOWS_EPOCH_OFFSET = 116444736000000000ULL; // microseconds
    return (uli.QuadPart / 10) - WINDOWS_EPOCH_OFFSET;
}

FILETIME TimestampToolbox::UnixMicrosecondsToFileTime(uint64_t microseconds) {
    FILETIME ft;
    const uint64_t WINDOWS_EPOCH_OFFSET = 116444736000000000ULL; // microseconds
    uint64_t windows_microseconds = microseconds + WINDOWS_EPOCH_OFFSET;

    ULARGE_INTEGER uli;
    uli.QuadPart = windows_microseconds * 10; // Convert to 100-nanosecond intervals

    ft.dwLowDateTime = uli.LowPart;
    ft.dwHighDateTime = uli.HighPart;

    return ft;
}

void TimestampToolbox::UnixTimeToTm(time_t unix_seconds, tm& tm_time) {
#ifdef _WIN32
    errno_t error = localtime_s(&tm_time, &unix_seconds);
    if (error != 0) {
        // Handle error - set to epoch
        memset(&tm_time, 0, sizeof(tm));
    }
#else
    localtime_r(&unix_seconds, &tm_time);
#endif
}

time_t TimestampToolbox::TmToUnixTime(const tm& tm_time) {
#ifdef _WIN32
    tm tm_time_copy = tm_time; // Create a copy for mktime
    return mktime(&tm_time_copy);
#else
    tm tm_time_copy = tm_time; // Create a copy for timegm
    return timegm(&tm_time_copy);
#endif
}

bool TimestampToolbox::IsLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int TimestampToolbox::GetDaysInMonth(int month, int year) {
    static const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month < 1 || month > 12) return 0;

    int days = days_in_month[month - 1];
    if (month == 2 && IsLeapYear(year)) {
        days++;
    }

    return days;
}