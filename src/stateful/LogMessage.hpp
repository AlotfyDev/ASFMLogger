#ifndef __ASFM_LOGGER_MESSAGE_HPP__
#define __ASFM_LOGGER_MESSAGE_HPP__

/**
 * ASFMLogger Stateful LogMessage
 *
 * Stateful wrapper class that uses LogMessageToolbox internally.
 * Provides object-oriented interface while maintaining pure logic separation.
 */

#include "structs/LogDataStructures.hpp"
#include "toolbox/LogMessageToolbox.hpp"
#include <string>
#include <memory>

class LogMessage {
private:
    LogMessageData data_;  // POD data only

public:
    // =================================================================================
    // CONSTRUCTORS AND FACTORY METHODS
    // =================================================================================

    /**
     * @brief Default constructor
     */
    LogMessage();

    /**
     * @brief Constructor with message details
     * @param type Message type
     * @param message Message content
     * @param component Component name (optional)
     * @param function Function name (optional)
     * @param file Source file (optional)
     * @param line Source line number (optional)
     */
    LogMessage(LogMessageType type, const std::string& message,
               const std::string& component = "", const std::string& function = "",
               const std::string& file = "", uint32_t line = 0);

    /**
     * @brief Constructor from existing LogMessageData
     * @param data Existing message data
     */
    explicit LogMessage(const LogMessageData& data);

    /**
     * @brief Copy constructor
     * @param other Message to copy from
     */
    LogMessage(const LogMessage& other);

    /**
     * @brief Move constructor
     * @param other Message to move from
     */
    LogMessage(LogMessage&& other) noexcept;

    /**
     * @brief Assignment operator
     * @param other Message to assign from
     * @return Reference to this message
     */
    LogMessage& operator=(const LogMessage& other);

    /**
     * @brief Move assignment operator
     * @param other Message to move from
     * @return Reference to this message
     */
    LogMessage& operator=(LogMessage&& other) noexcept;

    /**
     * @brief Destructor
     */
    ~LogMessage() = default;

    // =================================================================================
    // FACTORY METHODS
    // =================================================================================

    /**
     * @brief Create a new message using toolbox
     * @param type Message type
     * @param message Message content
     * @param component Component name (optional)
     * @param function Function name (optional)
     * @param file Source file (optional)
     * @param line Source line number (optional)
     * @return New LogMessage instance
     */
    static LogMessage Create(LogMessageType type, const std::string& message,
                            const std::string& component = "", const std::string& function = "",
                            const std::string& file = "", uint32_t line = 0);

    /**
     * @brief Create a copy of this message
     * @return New LogMessage instance
     */
    LogMessage clone() const;

    // =================================================================================
    // GETTERS (Using toolbox internally)
    // =================================================================================

    /**
     * @brief Get message content
     * @return Message string
     */
    std::string getMessage() const {
        return LogMessageToolbox::ExtractMessage(data_);
    }

    /**
     * @brief Get component name
     * @return Component string
     */
    std::string getComponent() const {
        return LogMessageToolbox::ExtractComponent(data_);
    }

    /**
     * @brief Get function name
     * @return Function string
     */
    std::string getFunction() const {
        return LogMessageToolbox::ExtractFunction(data_);
    }

    /**
     * @brief Get source file
     * @return Source file string
     */
    std::string getFile() const {
        return LogMessageToolbox::ExtractFile(data_);
    }

    /**
     * @brief Get message type
     * @return Message type
     */
    LogMessageType getType() const {
        return LogMessageToolbox::GetType(data_);
    }

    /**
     * @brief Get message ID
     * @return Message ID
     */
    uint32_t getId() const {
        return LogMessageToolbox::GetId(data_);
    }

    /**
     * @brief Get instance ID
     * @return Instance ID
     */
    uint32_t getInstanceId() const {
        return LogMessageToolbox::GetInstanceId(data_);
    }

    /**
     * @brief Get timestamp
     * @return Message timestamp
     */
    LogTimestamp getTimestamp() const {
        return data_.timestamp;
    }

    /**
     * @brief Get process ID
     * @return Process ID
     */
    DWORD getProcessId() const {
        return data_.process_id;
    }

    /**
     * @brief Get thread ID
     * @return Thread ID
     */
    DWORD getThreadId() const {
        return data_.thread_id;
    }

    /**
     * @brief Get line number
     * @return Source line number
     */
    uint32_t getLineNumber() const {
        return data_.line_number;
    }

    /**
     * @brief Get raw data structure (const access)
     * @return Const reference to internal data
     */
    const LogMessageData& getData() const {
        return data_;
    }

    // =================================================================================
    // SETTERS (Using toolbox internally)
    // =================================================================================

    /**
     * @brief Set message content
     * @param message New message content
     * @return true if successful
     */
    bool setMessage(const std::string& message) {
        return LogMessageToolbox::SetMessage(data_, message);
    }

    /**
     * @brief Set component name
     * @param component Component name
     * @return true if successful
     */
    bool setComponent(const std::string& component) {
        return LogMessageToolbox::SetComponent(data_, component);
    }

    /**
     * @brief Set function name
     * @param function Function name
     * @return true if successful
     */
    bool setFunction(const std::string& function) {
        return LogMessageToolbox::SetFunction(data_, function);
    }

    /**
     * @brief Set source location
     * @param file Source file name
     * @param line Source line number
     * @return true if successful
     */
    bool setSourceLocation(const std::string& file, uint32_t line) {
        return LogMessageToolbox::SetSourceLocation(data_, file, line);
    }

    /**
     * @brief Set message type
     * @param type New message type
     */
    void setType(LogMessageType type) {
        LogMessageToolbox::SetType(data_, type);
    }

    // =================================================================================
    // VALIDATION AND INSPECTION
    // =================================================================================

    /**
     * @brief Check if message is valid
     * @return true if message is valid
     */
    bool isValid() const {
        return LogMessageToolbox::ValidateMessage(data_);
    }

    /**
     * @brief Check if message has content
     * @return true if message has content
     */
    bool hasContent() const {
        return LogMessageToolbox::HasContent(data_);
    }

    /**
     * @brief Check if message has component information
     * @return true if component is set
     */
    bool hasComponent() const {
        return LogMessageToolbox::HasComponent(data_);
    }

    // =================================================================================
    // FORMATTING AND OUTPUT
    // =================================================================================

    /**
     * @brief Convert to string representation
     * @return Human-readable string
     */
    std::string toString() const {
        return LogMessageToolbox::MessageToString(data_);
    }

    /**
     * @brief Convert to JSON format
     * @return JSON string representation
     */
    std::string toJson() const {
        return LogMessageToolbox::MessageToJson(data_);
    }

    /**
     * @brief Convert to CSV format
     * @return CSV string representation
     */
    std::string toCsv() const {
        return LogMessageToolbox::MessageToCsv(data_);
    }

    /**
     * @brief Format for console output
     * @return Console-formatted string
     */
    std::string formatForConsole() const {
        return LogMessageToolbox::FormatForConsole(data_);
    }

    // =================================================================================
    // COMPARISON OPERATORS
    // =================================================================================

    /**
     * @brief Equality operator
     * @param other Message to compare with
     * @return true if messages are equal
     */
    bool operator==(const LogMessage& other) const {
        return LogMessageToolbox::CompareMessages(data_, other.data_);
    }

    /**
     * @brief Inequality operator
     * @param other Message to compare with
     * @return true if messages are not equal
     */
    bool operator!=(const LogMessage& other) const {
        return !(*this == other);
    }

    /**
     * @brief Less than operator (by timestamp)
     * @param other Message to compare with
     * @return true if this message is earlier than other
     */
    bool operator<(const LogMessage& other) const;

    /**
     * @brief Greater than operator (by timestamp)
     * @param other Message to compare with
     * @return true if this message is later than other
     */
    bool operator>(const LogMessage& other) const;

    // =================================================================================
    // UTILITY METHODS
    // =================================================================================

    /**
     * @brief Create a hash of the message content
     * @return Hash value for content-based operations
     */
    uint32_t hashContent() const {
        return LogMessageToolbox::HashMessageContent(data_);
    }

    /**
     * @brief Swap contents with another message
     * @param other Message to swap with
     */
    void swap(LogMessage& other) noexcept;

    /**
     * @brief Clear message content
     */
    void clear();

    /**
     * @brief Check if message is empty
     * @return true if message has no content
     */
    bool empty() const {
        return !hasContent();
    }
};

// =====================================================================================
// NON-MEMBER FUNCTIONS
// =====================================================================================

/**
 * @brief Swap two LogMessage objects
 * @param a First message
 * @param b Second message
 */
void swap(LogMessage& a, LogMessage& b) noexcept;

/**
 * @brief Output stream operator for LogMessage
 * @param os Output stream
 * @param message Message to output
 * @return Reference to output stream
 */
std::ostream& operator<<(std::ostream& os, const LogMessage& message);

#endif // __ASFM_LOGGER_MESSAGE_HPP__