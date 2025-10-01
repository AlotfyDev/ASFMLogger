/**
 * ASFMLogger C# Wrapper
 *
 * This class provides a C# interface to the ASFMLogger C++ library,
 * enabling .NET applications to use the enhanced logging capabilities.
 */

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.IO;
using Newtonsoft.Json;

namespace ASFMLogger.Wrapper
{
    /// <summary>
    /// Log levels matching the C++ LogMessageType enum
    /// </summary>
    public enum LogLevel
    {
        TRACE = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        CRITICAL = 5
    }

    /// <summary>
    /// Message importance levels matching the C++ MessageImportance enum
    /// </summary>
    public enum MessageImportance
    {
        LOW = 0,
        MEDIUM = 1,
        HIGH = 2,
        CRITICAL = 3
    }

    /// <summary>
    /// C# wrapper for ASFMLogger C++ library
    /// </summary>
    public class ASFMLogger : IDisposable
    {
        private IntPtr _loggerInstance = IntPtr.Zero;
        private bool _disposed = false;
        private string _applicationName;
        private string _processName;
        private List<LogMessage> _localLogQueue;
        private readonly object _queueLock = new object();
        private bool _enhancedFeaturesEnabled = false;

        // C++ library imports
        private const string DllName = "ASFMLogger.dll";

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern IntPtr getInstance([MarshalAs(UnmanagedType.LPStr)] string applicationName,
                                                [MarshalAs(UnmanagedType.LPStr)] string processName);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern void log(IntPtr logger, [MarshalAs(UnmanagedType.LPStr)] string level,
                                     [MarshalAs(UnmanagedType.LPStr)] string component,
                                     [MarshalAs(UnmanagedType.LPStr)] string function,
                                     [MarshalAs(UnmanagedType.LPStr)] string message);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern void configureEnhanced(IntPtr logger,
                                                    [MarshalAs(UnmanagedType.LPStr)] string applicationName,
                                                    bool enableDatabase,
                                                    [MarshalAs(UnmanagedType.LPStr)] string databaseConnection,
                                                    bool enableSharedMemory,
                                                    [MarshalAs(UnmanagedType.LPStr)] string sharedMemoryName,
                                                    bool consoleOutput,
                                                    [MarshalAs(UnmanagedType.LPStr)] string logFileName,
                                                    ulong maxFileSize,
                                                    ulong maxFiles,
                                                    int logLevel);

        /// <summary>
        /// Log message structure for local queue
        /// </summary>
        public class LogMessage
        {
            public DateTime Timestamp { get; set; }
            public LogLevel Level { get; set; }
            public string Component { get; set; }
            public string Function { get; set; }
            public string Message { get; set; }
            public MessageImportance Importance { get; set; }
        }

        /// <summary>
        /// Initialize ASFMLogger with application and process names
        /// </summary>
        public ASFMLogger(string applicationName = "CSharpApp", string processName = "")
        {
            _applicationName = applicationName;
            _processName = processName ?? $"CSharp_{Thread.CurrentThread.ManagedThreadId}";
            _localLogQueue = new List<LogMessage>();

            try
            {
                // Try to load C++ library and get instance
                _loggerInstance = getInstance(_applicationName, _processName);
                if (_loggerInstance != IntPtr.Zero)
                {
                    _enhancedFeaturesEnabled = true;
                    Console.WriteLine($"Enhanced ASFMLogger initialized for {_applicationName}");
                }
                else
                {
                    Console.WriteLine("Warning: C++ ASFMLogger not available. Using local-only mode.");
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Warning: Failed to initialize C++ ASFMLogger: {ex.Message}");
                Console.WriteLine("Using local-only mode with limited functionality.");
            }
        }

        /// <summary>
        /// Log a trace message
        /// </summary>
        public void Trace(string message, string component = "CSharp", string function = "")
        {
            Log(LogLevel.TRACE, message, component, function);
        }

        /// <summary>
        /// Log a debug message
        /// </summary>
        public void Debug(string message, string component = "CSharp", string function = "")
        {
            Log(LogLevel.DEBUG, message, component, function);
        }

        /// <summary>
        /// Log an info message
        /// </summary>
        public void Info(string message, string component = "CSharp", string function = "")
        {
            Log(LogLevel.INFO, message, component, function);
        }

        /// <summary>
        /// Log a warning message
        /// </summary>
        public void Warn(string message, string component = "CSharp", string function = "")
        {
            Log(LogLevel.WARN, message, component, function);
        }

        /// <summary>
        /// Log an error message
        /// </summary>
        public void Error(string message, string component = "CSharp", string function = "")
        {
            Log(LogLevel.ERROR, message, component, function);
        }

        /// <summary>
        /// Log a critical message
        /// </summary>
        public void Critical(string message, string component = "CSharp", string function = "")
        {
            Log(LogLevel.CRITICAL, message, component, function);
        }

        /// <summary>
        /// Internal logging method with component tracking
        /// </summary>
        private void Log(LogLevel level, string message, string component, string function)
        {
            try
            {
                string formattedMessage = $"{DateTime.Now:yyyy-MM-dd HH:mm:ss.fff} [{component}] {message}";

                // Use C++ enhanced logging if available
                if (_enhancedFeaturesEnabled && _loggerInstance != IntPtr.Zero)
                {
                    try
                    {
                        string levelStr = level.ToString().ToLower();
                        log(_loggerInstance, levelStr, component, function, formattedMessage);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"Warning: C++ logging failed: {ex.Message}");
                    }
                }

                // Always maintain local queue for C# analysis
                lock (_queueLock)
                {
                    var logMessage = new LogMessage
                    {
                        Timestamp = DateTime.Now,
                        Level = level,
                        Component = component,
                        Function = function,
                        Message = message,
                        Importance = DetermineImportance(level)
                    };

                    _localLogQueue.Add(logMessage);

                    // Keep only last 1000 messages
                    if (_localLogQueue.Count > 1000)
                    {
                        _localLogQueue.RemoveAt(0);
                    }
                }

                // Output to console
                OutputToConsole(level, formattedMessage);

            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error in C# logging: {ex.Message}");
            }
        }

        /// <summary>
        /// Determine message importance based on log level
        /// </summary>
        private MessageImportance DetermineImportance(LogLevel level)
        {
            switch (level)
            {
                case LogLevel.TRACE:
                case LogLevel.DEBUG:
                    return MessageImportance.LOW;
                case LogLevel.INFO:
                    return MessageImportance.MEDIUM;
                case LogLevel.WARN:
                    return MessageImportance.HIGH;
                case LogLevel.ERROR:
                case LogLevel.CRITICAL:
                    return MessageImportance.CRITICAL;
                default:
                    return MessageImportance.MEDIUM;
            }
        }

        /// <summary>
        /// Output message to console with appropriate formatting
        /// </summary>
        private void OutputToConsole(LogLevel level, string message)
        {
            ConsoleColor originalColor = Console.ForegroundColor;

            try
            {
                switch (level)
                {
                    case LogLevel.TRACE:
                        Console.ForegroundColor = ConsoleColor.Gray;
                        Console.WriteLine($"TRACE: {message}");
                        break;
                    case LogLevel.DEBUG:
                        Console.ForegroundColor = ConsoleColor.Cyan;
                        Console.WriteLine($"DEBUG: {message}");
                        break;
                    case LogLevel.INFO:
                        Console.ForegroundColor = ConsoleColor.White;
                        Console.WriteLine($"INFO: {message}");
                        break;
                    case LogLevel.WARN:
                        Console.ForegroundColor = ConsoleColor.Yellow;
                        Console.WriteLine($"WARN: {message}");
                        break;
                    case LogLevel.ERROR:
                        Console.ForegroundColor = ConsoleColor.Red;
                        Console.WriteLine($"ERROR: {message}");
                        break;
                    case LogLevel.CRITICAL:
                        Console.ForegroundColor = ConsoleColor.DarkRed;
                        Console.WriteLine($"CRITICAL: {message}");
                        break;
                }
            }
            finally
            {
                Console.ForegroundColor = originalColor;
            }
        }

        /// <summary>
        /// Configure enhanced logging features
        /// </summary>
        public void ConfigureEnhanced(
            bool enableDatabase = false,
            string databaseConnection = "",
            bool enableSharedMemory = false,
            string sharedMemoryName = "",
            bool consoleOutput = true,
            string logFileName = "csharp_app.log",
            ulong maxFileSize = 10485760, // 10 MB
            ulong maxFiles = 5,
            LogLevel minLogLevel = LogLevel.INFO)
        {
            try
            {
                if (_loggerInstance != IntPtr.Zero)
                {
                    configureEnhanced(
                        _loggerInstance,
                        _applicationName,
                        enableDatabase,
                        databaseConnection,
                        enableSharedMemory,
                        sharedMemoryName,
                        consoleOutput,
                        logFileName,
                        maxFileSize,
                        maxFiles,
                        (int)minLogLevel
                    );

                    Console.WriteLine($"Enhanced ASFMLogger configured for {_applicationName}");
                }
                else
                {
                    Console.WriteLine("Warning: C++ library not available. Enhanced features disabled.");
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Warning: Failed to configure enhanced features: {ex.Message}");
            }
        }

        /// <summary>
        /// Get recent log messages from local C# queue
        /// </summary>
        public List<LogMessage> GetLocalLogs(string component = "", LogLevel? level = null, int limit = 100)
        {
            lock (_queueLock)
            {
                var filteredLogs = _localLogQueue.Where(log =>
                    (string.IsNullOrEmpty(component) || log.Component == component) &&
                    (!level.HasValue || log.Level == level.Value)
                ).Take(limit).ToList();

                return filteredLogs;
            }
        }

        /// <summary>
        /// Get logging statistics
        /// </summary>
        public Dictionary<string, object> GetLogStatistics()
        {
            lock (_queueLock)
            {
                if (!_localLogQueue.Any())
                {
                    return new Dictionary<string, object> { { "total_messages", 0 } };
                }

                var levelCounts = _localLogQueue
                    .GroupBy(log => log.Level)
                    .ToDictionary(group => group.Key.ToString(), group => group.Count());

                var componentCounts = _localLogQueue
                    .GroupBy(log => log.Component)
                    .ToDictionary(group => group.Key, group => group.Count());

                var oldest = _localLogQueue.Min(log => log.Timestamp);
                var newest = _localLogQueue.Max(log => log.Timestamp);
                var timeRange = (newest - oldest).TotalSeconds;

                return new Dictionary<string, object>
                {
                    { "total_messages", _localLogQueue.Count },
                    { "level_distribution", levelCounts },
                    { "component_distribution", componentCounts },
                    { "time_range_seconds", timeRange },
                    { "messages_per_second", timeRange > 0 ? _localLogQueue.Count / timeRange : 0 }
                };
            }
        }

        /// <summary>
        /// Export logs to JSON file
        /// </summary>
        public void ExportLogsToJson(string filePath)
        {
            try
            {
                lock (_queueLock)
                {
                    var json = JsonConvert.SerializeObject(_localLogQueue, Formatting.Indented);
                    File.WriteAllText(filePath, json);
                }
                Console.WriteLine($"Exported {_localLogQueue.Count} log messages to {filePath}");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error exporting logs: {ex.Message}");
            }
        }

        /// <summary>
        /// Export logs to CSV file
        /// </summary>
        public void ExportLogsToCsv(string filePath)
        {
            try
            {
                lock (_queueLock)
                {
                    using (var writer = new StreamWriter(filePath))
                    {
                        writer.WriteLine("Timestamp,Level,Component,Function,Message,Importance");

                        foreach (var log in _localLogQueue)
                        {
                            writer.WriteLine($"\"{log.Timestamp:yyyy-MM-dd HH:mm:ss.fff}\",\"{log.Level}\",\"{log.Component}\",\"{log.Function}\",\"{log.Message}\",\"{log.Importance}\"");
                        }
                    }
                }
                Console.WriteLine($"Exported {_localLogQueue.Count} log messages to {filePath}");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error exporting logs: {ex.Message}");
            }
        }

        /// <summary>
        /// Clear local log queue
        /// </summary>
        public void ClearLocalLogs()
        {
            lock (_queueLock)
            {
                _localLogQueue.Clear();
            }
        }

        /// <summary>
        /// Get application name
        /// </summary>
        public string GetApplicationName()
        {
            return _applicationName;
        }

        /// <summary>
        /// Get process name
        /// </summary>
        public string GetProcessName()
        {
            return _processName;
        }

        /// <summary>
        /// Check if enhanced features are available
        /// </summary>
        public bool IsEnhancedFeaturesAvailable()
        {
            return _enhancedFeaturesEnabled;
        }

        /// <summary>
        /// Dispose of the logger and clean up resources
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Protected dispose method
        /// </summary>
        protected virtual void Dispose(bool disposing)
        {
            if (!_disposed)
            {
                if (disposing)
                {
                    // Clean up managed resources
                    if (_localLogQueue != null)
                    {
                        _localLogQueue.Clear();
                        _localLogQueue = null;
                    }
                }

                // Clean up unmanaged resources
                if (_loggerInstance != IntPtr.Zero)
                {
                    // C++ cleanup would go here if needed
                    _loggerInstance = IntPtr.Zero;
                }

                _disposed = true;
            }
        }

        /// <summary>
        /// Destructor
        /// </summary>
        ~ASFMLogger()
        {
            Dispose(false);
        }
    }

    /// <summary>
    /// Static convenience class for easy logging
    /// </summary>
    public static class ASFMLoggerStatic
    {
        private static ASFMLogger _globalLogger;

        /// <summary>
        /// Get or create global logger instance
        /// </summary>
        public static ASFMLogger GetGlobalLogger(string applicationName = "CSharpApp")
        {
            if (_globalLogger == null)
            {
                _globalLogger = new ASFMLogger(applicationName);
            }
            return _globalLogger;
        }

        /// <summary>
        /// Quick trace logging
        /// </summary>
        public static void Trace(string message, string component = "CSharp")
        {
            GetGlobalLogger().Trace(message, component);
        }

        /// <summary>
        /// Quick debug logging
        /// </summary>
        public static void Debug(string message, string component = "CSharp")
        {
            GetGlobalLogger().Debug(message, component);
        }

        /// <summary>
        /// Quick info logging
        /// </summary>
        public static void Info(string message, string component = "CSharp")
        {
            GetGlobalLogger().Info(message, component);
        }

        /// <summary>
        /// Quick warning logging
        /// </summary>
        public static void Warn(string message, string component = "CSharp")
        {
            GetGlobalLogger().Warn(message, component);
        }

        /// <summary>
        /// Quick error logging
        /// </summary>
        public static void Error(string message, string component = "CSharp")
        {
            GetGlobalLogger().Error(message, component);
        }

        /// <summary>
        /// Quick critical logging
        /// </summary>
        public static void Critical(string message, string component = "CSharp")
        {
            GetGlobalLogger().Critical(message, component);
        }

        /// <summary>
        /// Configure global logger
        /// </summary>
        public static void ConfigureGlobalLogger(
            bool enableDatabase = false,
            string databaseConnection = "",
            string logFileName = "csharp_app.log")
        {
            GetGlobalLogger().ConfigureEnhanced(
                enableDatabase: enableDatabase,
                databaseConnection: databaseConnection,
                logFileName: logFileName
            );
        }
    }
}