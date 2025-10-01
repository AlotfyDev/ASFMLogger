#ifndef __ASFM_LOGGER_CROSS_LANGUAGE_SERIALIZATION_HPP__
#define __ASFM_LOGGER_CROSS_LANGUAGE_SERIALIZATION_HPP__

/**
 * ASFMLogger Cross-Language Serialization Utilities
 *
 * Utilities for serializing log messages between different programming languages
 * and platforms, enabling seamless integration with Python, C#, MQL5, and web interfaces.
 */

#include "structs/LogDataStructures.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

// Forward declarations
struct LogMessageData;

class CrossLanguageSerialization {
public:
    // =================================================================================
    // JSON SERIALIZATION
    // =================================================================================

    /**
     * @brief Convert LogMessageData to JSON string
     * @param message Message to serialize
     * @param pretty_print Whether to format JSON for readability
     * @return JSON string representation
     */
    static std::string MessageToJson(const LogMessageData& message, bool pretty_print = false);

    /**
     * @brief Convert vector of messages to JSON array
     * @param messages Vector of messages to serialize
     * @param pretty_print Whether to format JSON for readability
     * @return JSON array string
     */
    static std::string MessagesToJson(const std::vector<LogMessageData>& messages, bool pretty_print = false);

    /**
     * @brief Parse LogMessageData from JSON string
     * @param json_string JSON string to parse
     * @return Parsed message data
     */
    static LogMessageData JsonToMessage(const std::string& json_string);

    /**
     * @brief Parse vector of messages from JSON array
     * @param json_string JSON array string to parse
     * @return Vector of parsed messages
     */
    static std::vector<LogMessageData> JsonToMessages(const std::string& json_string);

    // =================================================================================
    // PYTHON-SPECIFIC SERIALIZATION
    // =================================================================================

    /**
     * @brief Convert LogMessageData to Python dictionary string
     * @param message Message to serialize
     * @return Python dictionary representation as string
     */
    static std::string MessageToPythonDict(const LogMessageData& message);

    /**
     * @brief Convert vector of messages to Python list
     * @param messages Vector of messages to serialize
     * @return Python list representation as string
     */
    static std::string MessagesToPythonList(const std::vector<LogMessageData>& messages);

    /**
     * @brief Generate Python import statements for ASFMLogger
     * @param enable_enhanced Whether to include enhanced features
     * @return Python import statements
     */
    static std::string GeneratePythonImports(bool enable_enhanced = true);

    /**
     * @brief Generate Python class definition for LogMessage
     * @return Python class definition string
     */
    static std::string GeneratePythonLogMessageClass();

    // =================================================================================
    // C#-SPECIFIC SERIALIZATION
    // =================================================================================

    /**
     * @brief Convert LogMessageData to C# object initialization
     * @param message Message to serialize
     * @param variable_name Variable name for the object
     * @return C# object initialization code
     */
    static std::string MessageToCSharpObject(const LogMessageData& message, const std::string& variable_name);

    /**
     * @brief Convert vector of messages to C# list initialization
     * @param messages Vector of messages to serialize
     * @param list_name Name for the C# list
     * @return C# list initialization code
     */
    static std::string MessagesToCSharpList(const std::vector<LogMessageData>& messages, const std::string& list_name);

    /**
     * @brief Generate C# using statements for ASFMLogger
     * @return C# using statements
     */
    static std::string GenerateCSharpUsings();

    /**
     * @brief Generate C# enum definitions
     * @return C# enum definitions string
     */
    static std::string GenerateCSharpEnums();

    // =================================================================================
    // MQL5-SPECIFIC SERIALIZATION
    // =================================================================================

    /**
     * @brief Convert LogMessageData to MQL5 struct initialization
     * @param message Message to serialize
     * @param struct_name Name for the MQL5 struct
     * @return MQL5 struct initialization code
     */
    static std::string MessageToMQL5Struct(const LogMessageData& message, const std::string& struct_name);

    /**
     * @brief Convert vector of messages to MQL5 array
     * @param messages Vector of messages to serialize
     * @param array_name Name for the MQL5 array
     * @return MQL5 array initialization code
     */
    static std::string MessagesToMQL5Array(const std::vector<LogMessageData>& messages, const std::string& array_name);

    /**
     * @brief Generate MQL5 import statements for ASFMLogger DLL
     * @return MQL5 import statements
     */
    static std::string GenerateMQL5Imports();

    /**
     * @brief Generate MQL5 struct definitions
     * @return MQL5 struct definitions string
     */
    static std::string GenerateMQL5Structs();

    // =================================================================================
    // WEB/REST API SERIALIZATION
    // =================================================================================

    /**
     * @brief Convert LogMessageData to REST API format
     * @param message Message to serialize
     * @return Dictionary for REST API transmission
     */
    static std::unordered_map<std::string, std::string> MessageToRestApi(const LogMessageData& message);

    /**
     * @brief Convert vector of messages to REST API batch format
     * @param messages Vector of messages to serialize
     * @return Vector of dictionaries for REST API transmission
     */
    static std::vector<std::unordered_map<std::string, std::string>> MessagesToRestApiBatch(
        const std::vector<LogMessageData>& messages);

    /**
     * @brief Generate OpenAPI/Swagger specification for ASFMLogger API
     * @return OpenAPI specification as JSON string
     */
    static std::string GenerateOpenApiSpecification();

    /**
     * @brief Generate REST API endpoint documentation
     * @return REST API documentation as formatted string
     */
    static std::string GenerateRestApiDocumentation();

    // =================================================================================
    // WEBSOCKET SERIALIZATION
    // =================================================================================

    /**
     * @brief Convert LogMessageData to WebSocket message
     * @param message Message to serialize
     * @param event_type Event type for WebSocket
     * @return WebSocket message as JSON string
     */
    static std::string MessageToWebSocket(const LogMessageData& message, const std::string& event_type = "log_message");

    /**
     * @brief Convert vector of messages to WebSocket batch
     * @param messages Vector of messages to serialize
     * @param event_type Event type for WebSocket
     * @return WebSocket batch message as JSON string
     */
    static std::string MessagesToWebSocketBatch(const std::vector<LogMessageData>& messages,
                                               const std::string& event_type = "log_batch");

    /**
     * @brief Generate WebSocket client connection code (JavaScript)
     * @param server_url WebSocket server URL
     * @return JavaScript WebSocket client code
     */
    static std::string GenerateWebSocketClientCode(const std::string& server_url);

    /**
     * @brief Generate WebSocket server handler code (Node.js)
     * @return Node.js WebSocket server handler code
     */
    static std::string GenerateWebSocketServerCode();

    // =================================================================================
    // DATABASE SERIALIZATION
    // =================================================================================

    /**
     * @brief Convert LogMessageData to SQL INSERT statement
     * @param message Message to serialize
     * @param table_name Target table name
     * @param schema_name Schema name (optional)
     * @return SQL INSERT statement
     */
    static std::string MessageToSqlInsert(const LogMessageData& message,
                                         const std::string& table_name,
                                         const std::string& schema_name = "dbo");

    /**
     * @brief Convert vector of messages to SQL batch INSERT
     * @param messages Vector of messages to serialize
     * @param table_name Target table name
     * @param schema_name Schema name (optional)
     * @return SQL batch INSERT statement
     */
    static std::string MessagesToSqlBatchInsert(const std::vector<LogMessageData>& messages,
                                                const std::string& table_name,
                                                const std::string& schema_name = "dbo");

    /**
     * @brief Convert LogMessageData to MongoDB document
     * @param message Message to serialize
     * @param collection_name Target collection name
     * @return MongoDB document as JSON string
     */
    static std::string MessageToMongoDbDocument(const LogMessageData& message, const std::string& collection_name);

    /**
     * @brief Convert vector of messages to MongoDB batch
     * @param messages Vector of messages to serialize
     * @param collection_name Target collection name
     * @return MongoDB batch insert as JSON string
     */
    static std::string MessagesToMongoDbBatch(const std::vector<LogMessageData>& messages,
                                             const std::string& collection_name);

    // =================================================================================
    // CROSS-PLATFORM MESSAGE CONVERSION
    // =================================================================================

    /**
     * @brief Convert LogMessageData to platform-specific format
     * @param message Message to convert
     * @param target_platform Target platform ("PYTHON", "CSHARP", "MQL5", "WEB", "DATABASE")
     * @return Platform-specific serialized string
     */
    static std::string MessageToPlatformFormat(const LogMessageData& message, const std::string& target_platform);

    /**
     * @brief Convert vector of messages to platform-specific batch format
     * @param messages Vector of messages to convert
     * @param target_platform Target platform
     * @return Platform-specific batch serialized string
     */
    static std::string MessagesToPlatformBatchFormat(const std::vector<LogMessageData>& messages,
                                                     const std::string& target_platform);

    /**
     * @brief Create platform-specific wrapper code
     * @param target_language Target programming language
     * @param include_enhanced_features Whether to include enhanced features
     * @return Wrapper code for the target language
     */
    static std::string CreatePlatformWrapper(const std::string& target_language, bool include_enhanced_features = true);

    /**
     * @brief Generate language-specific example usage
     * @param target_language Target programming language
     * @return Example usage code for the target language
     */
    static std::string GenerateUsageExample(const std::string& target_language);

    // =================================================================================
    // MESSAGE FILTERING FOR CROSS-LANGUAGE
    // =================================================================================

    /**
     * @brief Filter messages for specific platform requirements
     * @param messages Vector of messages to filter
     * @param target_platform Target platform
     * @param max_message_size Maximum message size for platform
     * @return Filtered vector of messages
     */
    static std::vector<LogMessageData> FilterForPlatform(const std::vector<LogMessageData>& messages,
                                                        const std::string& target_platform,
                                                        size_t max_message_size = 8192);

    /**
     * @brief Convert message encoding for target platform
     * @param message Message to convert
     * @param target_platform Target platform
     * @return Converted message data
     */
    static LogMessageData ConvertMessageEncoding(const LogMessageData& message, const std::string& target_platform);

    /**
     * @brief Validate message compatibility with target platform
     * @param message Message to validate
     * @param target_platform Target platform
     * @return true if message is compatible
     */
    static bool ValidatePlatformCompatibility(const LogMessageData& message, const std::string& target_platform);

    // =================================================================================
    // BATCH PROCESSING FOR CROSS-LANGUAGE
    // =================================================================================

    /**
     * @brief Create batch for cross-language transmission
     * @param messages Vector of messages to batch
     * @param target_platform Target platform
     * @param batch_size Maximum batch size
     * @return Vector of message batches
     */
    static std::vector<std::vector<LogMessageData>> CreateCrossLanguageBatches(
        const std::vector<LogMessageData>& messages,
        const std::string& target_platform,
        size_t batch_size = 100);

    /**
     * @brief Compress message data for efficient transmission
     * @param messages Vector of messages to compress
     * @return Compressed data as string
     */
    static std::string CompressMessages(const std::vector<LogMessageData>& messages);

    /**
     * @brief Decompress message data
     * @param compressed_data Compressed data string
     * @return Vector of decompressed messages
     */
    static std::vector<LogMessageData> DecompressMessages(const std::string& compressed_data);

    // =================================================================================
    // LANGUAGE BRIDGE PATTERNS
    // =================================================================================

    /**
     * @brief Generate C++ wrapper for Python ctypes
     * @return C++ wrapper code for Python integration
     */
    static std::string GeneratePythonCtypesWrapper();

    /**
     * @brief Generate C++ wrapper for C# P/Invoke
     * @return C++ wrapper code for C# integration
     */
    static std::string GenerateCsharpPInvokeWrapper();

    /**
     * @brief Generate C++ wrapper for MQL5 DLL import
     * @return C++ wrapper code for MQL5 integration
     */
    static std::string GenerateMQL5DllWrapper();

    /**
     * @brief Generate bridge interface for web APIs
     * @return C++ bridge code for web integration
     */
    static std::string GenerateWebApiBridge();

    // =================================================================================
    // UTILITY FUNCTIONS
    // =================================================================================

    /**
     * @brief Get platform-specific message size limits
     * @param platform Target platform
     * @return Maximum message size in bytes
     */
    static size_t GetPlatformMessageSizeLimit(const std::string& platform);

    /**
     * @brief Get platform-specific encoding requirements
     * @param platform Target platform
     * @return Encoding requirements as string
     */
    static std::string GetPlatformEncodingRequirements(const std::string& platform);

    /**
     * @brief Validate message for cross-language compatibility
     * @param message Message to validate
     * @return true if message is compatible with all platforms
     */
    static bool ValidateCrossLanguageCompatibility(const LogMessageData& message);

    /**
     * @brief Generate cross-language compatibility report
     * @param messages Vector of messages to analyze
     * @return Compatibility report as formatted string
     */
    static std::string GenerateCompatibilityReport(const std::vector<LogMessageData>& messages);

private:
    // Private helper methods
    static nlohmann::json MessageToJsonObject(const LogMessageData& message);
    static LogMessageData JsonObjectToMessage(const nlohmann::json& json_obj);
    static std::string EscapeForPythonString(const std::string& input);
    static std::string EscapeForCSharpString(const std::string& input);
    static std::string EscapeForMQL5String(const std::string& input);
    static std::string FormatTimestampForPlatform(const LogTimestamp& timestamp, const std::string& platform);
    static bool IsValidPlatformName(const std::string& platform);
};

#endif // __ASFM_LOGGER_CROSS_LANGUAGE_SERIALIZATION_HPP__