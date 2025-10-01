"""
ASFMLogger Python Wrapper

This module provides a Python interface to the ASFMLogger C++ library,
enabling Python applications to use the enhanced logging capabilities.
"""

import ctypes
import json
import os
import sys
import threading
import time
from datetime import datetime
from enum import Enum
from typing import Optional, Dict, Any, List
from pathlib import Path


class LogLevel(Enum):
    """Python equivalent of LogMessageType enum"""
    TRACE = 0
    DEBUG = 1
    INFO = 2
    WARN = 3
    ERROR = 4
    CRITICAL = 5


class MessageImportance(Enum):
    """Python equivalent of MessageImportance enum"""
    LOW = 0
    MEDIUM = 1
    HIGH = 2
    CRITICAL = 3


class ASFMLoggerPython:
    """
    Python wrapper for ASFMLogger C++ library.

    This class provides a Pythonic interface to the enhanced ASFMLogger
    while maintaining all the advanced features like importance-based
    routing, contextual persistence, and multi-instance management.
    """

    def __init__(self, application_name: str = "PythonApp", process_name: str = ""):
        """
        Initialize the Python ASFMLogger wrapper.

        Args:
            application_name: Name of the Python application
            process_name: Name of the Python process (optional)
        """
        self.application_name = application_name
        self.process_name = process_name or f"Python_{os.getpid()}"
        self._logger = None
        self._library_loaded = False
        self._local_queue = []
        self._queue_lock = threading.Lock()

        # Load the C++ library
        self._load_cpp_library()

        # Initialize enhanced features if available
        self._initialize_enhanced_features()

    def _load_cpp_library(self):
        """Load the ASFMLogger C++ library"""
        try:
            # Try to load the compiled library
            library_paths = [
                "lib/ASFMLogger.dll",  # Windows
                "lib/libASFMLogger.so",  # Linux
                "lib/libASFMLogger.dylib",  # macOS
                "build/Release/ASFMLogger.dll",  # Visual Studio build
                "build/ASFMLogger.dll",  # MSVC build
            ]

            for path in library_paths:
                if os.path.exists(path):
                    self._cpp_library = ctypes.CDLL(path)
                    self._library_loaded = True
                    break

            if not self._library_loaded:
                print("Warning: ASFMLogger C++ library not found. Running in local-only mode.")
                return

            # Set up function signatures for enhanced features
            self._setup_cpp_function_signatures()

        except Exception as e:
            print(f"Warning: Failed to load C++ library: {e}")
            print("Running in local-only mode with limited functionality.")

    def _setup_cpp_function_signatures(self):
        """Set up ctypes function signatures for C++ library interface"""
        try:
            # Basic logger functions
            self._cpp_library.getInstance.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
            self._cpp_library.getInstance.restype = ctypes.c_void_p

            # Enhanced logging functions
            self._cpp_library.log.argtypes = [
                ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p,
                ctypes.c_char_p, ctypes.c_char_p
            ]
            self._cpp_library.log.restype = None

            # Configuration functions
            self._cpp_library.configureEnhanced.argtypes = [
                ctypes.c_void_p, ctypes.c_char_p, ctypes.c_bool,
                ctypes.c_char_p, ctypes.c_bool, ctypes.c_char_p,
                ctypes.c_bool, ctypes.c_char_p, ctypes.c_size_t,
                ctypes.c_size_t, ctypes.c_int
            ]
            self._cpp_library.configureEnhanced.restype = None

        except Exception as e:
            print(f"Warning: Failed to setup C++ function signatures: {e}")

    def _initialize_enhanced_features(self):
        """Initialize enhanced logging features"""
        if not self._library_loaded:
            return

        try:
            # Get C++ logger instance
            app_name_bytes = self.application_name.encode('utf-8')
            proc_name_bytes = self.process_name.encode('utf-8')

            self._logger = self._cpp_library.getInstance(app_name_bytes, proc_name_bytes)

            if self._logger:
                print(f"Enhanced ASFMLogger initialized for {self.application_name}")

        except Exception as e:
            print(f"Warning: Failed to initialize enhanced features: {e}")

    def trace(self, message: str, component: str = "Python", function: str = ""):
        """Log a trace message with component tracking"""
        self._log_with_component("trace", message, component, function)

    def debug(self, message: str, component: str = "Python", function: str = ""):
        """Log a debug message with component tracking"""
        self._log_with_component("debug", message, component, function)

    def info(self, message: str, component: str = "Python", function: str = ""):
        """Log an info message with component tracking"""
        self._log_with_component("info", message, component, function)

    def warn(self, message: str, component: str = "Python", function: str = ""):
        """Log a warning message with component tracking"""
        self._log_with_component("warn", message, component, function)

    def error(self, message: str, component: str = "Python", function: str = ""):
        """Log an error message with component tracking"""
        self._log_with_component("error", message, component, function)

    def critical(self, message: str, component: str = "Python", function: str = ""):
        """Log a critical message with component tracking"""
        self._log_with_component("critical", message, component, function)

    def _log_with_component(self, level: str, message: str, component: str, function: str):
        """Internal method to log with component information"""
        try:
            # Format message with timestamp
            timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
            formatted_message = f"[{timestamp}] [{component}] {message}"

            # Use C++ enhanced logging if available
            if self._logger and self._library_loaded:
                try:
                    level_bytes = level.encode('utf-8')
                    component_bytes = component.encode('utf-8')
                    function_bytes = function.encode('utf-8')
                    message_bytes = formatted_message.encode('utf-8')

                    self._cpp_library.log(
                        self._logger, level_bytes, component_bytes,
                        function_bytes, message_bytes
                    )
                except:
                    # Fallback to basic logging
                    pass

            # Always maintain local queue for Python analysis
            with self._queue_lock:
                self._local_queue.append({
                    'timestamp': timestamp,
                    'level': level.upper(),
                    'component': component,
                    'function': function,
                    'message': message,
                    'formatted_message': formatted_message
                })

                # Keep only last 1000 messages in local queue
                if len(self._local_queue) > 1000:
                    self._local_queue.pop(0)

            # Also output to Python logging
            self._output_to_python_logging(level, formatted_message)

        except Exception as e:
            print(f"Error in Python logging: {e}")

    def _output_to_python_logging(self, level: str, message: str):
        """Output message to Python's logging system"""
        if level.upper() == "TRACE":
            print(f"TRACE: {message}")
        elif level.upper() == "DEBUG":
            print(f"DEBUG: {message}")
        elif level.upper() == "INFO":
            print(f"INFO: {message}")
        elif level.upper() == "WARN":
            print(f"WARNING: {message}")
        elif level.upper() == "ERROR":
            print(f"ERROR: {message}")
        elif level.upper() == "CRITICAL":
            print(f"CRITICAL: {message}")

    def configure_enhanced(self,
                          enable_database: bool = False,
                          database_connection: str = "",
                          enable_shared_memory: bool = False,
                          shared_memory_name: str = "",
                          console_output: bool = True,
                          log_file: str = "python_app.log",
                          max_file_size: int = 10485760,
                          max_files: int = 5,
                          min_log_level: str = "INFO"):
        """
        Configure enhanced logging features.

        Args:
            enable_database: Whether to enable database logging
            database_connection: Database connection string
            enable_shared_memory: Whether to enable shared memory
            shared_memory_name: Name for shared memory region
            console_output: Whether to enable console output
            log_file: Path to log file
            max_file_size: Maximum log file size in bytes
            max_files: Maximum number of log files to keep
            min_log_level: Minimum log level ("TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL")
        """
        try:
            if self._logger and self._library_loaded:
                # Convert log level string to enum value
                level_map = {
                    "TRACE": 0, "DEBUG": 1, "INFO": 2,
                    "WARN": 3, "ERROR": 4, "CRITICAL": 5
                }
                log_level_value = level_map.get(min_log_level.upper(), 2)

                # Call C++ configuration function
                self._cpp_library.configureEnhanced(
                    self._logger,
                    self.application_name.encode('utf-8'),
                    enable_database,
                    database_connection.encode('utf-8'),
                    enable_shared_memory,
                    shared_memory_name.encode('utf-8'),
                    console_output,
                    log_file.encode('utf-8'),
                    max_file_size,
                    max_files,
                    log_level_value
                )

            print(f"Enhanced ASFMLogger configured for {self.application_name}")

        except Exception as e:
            print(f"Warning: Failed to configure enhanced features: {e}")

    def get_local_logs(self, component: str = "", level: str = "", limit: int = 100) -> List[Dict]:
        """
        Get recent log messages from local Python queue.

        Args:
            component: Filter by component name (optional)
            level: Filter by log level (optional)
            limit: Maximum number of messages to return

        Returns:
            List of log message dictionaries
        """
        with self._queue_lock:
            filtered_logs = self._local_queue.copy()

        # Apply filters
        if component:
            filtered_logs = [log for log in filtered_logs if log['component'] == component]

        if level:
            filtered_logs = [log for log in filtered_logs if log['level'] == level.upper()]

        # Return most recent messages
        return filtered_logs[-limit:] if filtered_logs else []

    def get_log_statistics(self) -> Dict[str, Any]:
        """Get logging statistics from local queue"""
        with self._queue_lock:
            logs = self._local_queue

        if not logs:
            return {"total_messages": 0}

        # Count by level
        level_counts = {}
        for log in logs:
            level = log['level']
            level_counts[level] = level_counts.get(level, 0) + 1

        # Count by component
        component_counts = {}
        for log in logs:
            component = log['component']
            component_counts[component] = component_counts.get(component, 0) + 1

        # Time range
        if len(logs) >= 2:
            oldest = datetime.strptime(logs[0]['timestamp'], "%Y-%m-%d %H:%M:%S.%f")
            newest = datetime.strptime(logs[-1]['timestamp'], "%Y-%m-%d %H:%M:%S.%f")
            time_range = (newest - oldest).total_seconds()
        else:
            time_range = 0

        return {
            "total_messages": len(logs),
            "level_distribution": level_counts,
            "component_distribution": component_counts,
            "time_range_seconds": time_range,
            "messages_per_second": len(logs) / max(time_range, 1)
        }

    def clear_local_logs(self):
        """Clear the local Python log queue"""
        with self._queue_lock:
            self._local_queue.clear()

    def export_logs_to_file(self, file_path: str, format: str = "json"):
        """
        Export local logs to file.

        Args:
            file_path: Path to export file
            format: Export format ("json", "csv", "txt")
        """
        with self._queue_lock:
            logs = self._local_queue.copy()

        try:
            if format.lower() == "json":
                with open(file_path, 'w') as f:
                    json.dump(logs, f, indent=2)

            elif format.lower() == "csv":
                import csv
                if logs:
                    with open(file_path, 'w', newline='') as f:
                        writer = csv.DictWriter(f, fieldnames=logs[0].keys())
                        writer.writeheader()
                        writer.writerows(logs)

            elif format.lower() == "txt":
                with open(file_path, 'w') as f:
                    for log in logs:
                        f.write(f"{log['timestamp']} [{log['level']}] [{log['component']}] {log['message']}\n")

            print(f"Exported {len(logs)} log messages to {file_path}")

        except Exception as e:
            print(f"Error exporting logs: {e}")

    def enable_real_time_monitoring(self, callback=None, interval_seconds: float = 1.0):
        """
        Enable real-time monitoring of log messages.

        Args:
            callback: Function to call with new messages
            interval_seconds: How often to check for new messages
        """
        def monitoring_loop():
            last_check_time = time.time()
            while True:
                try:
                    current_time = time.time()
                    if current_time - last_check_time >= interval_seconds:
                        # Get new messages since last check
                        with self._queue_lock:
                            new_messages = [log for log in self._local_queue
                                          if datetime.strptime(log['timestamp'], "%Y-%m-%d %H:%M:%S.%f") > datetime.fromtimestamp(last_check_time)]

                        if new_messages and callback:
                            callback(new_messages)

                        last_check_time = current_time

                    time.sleep(0.1)  # Small delay to prevent busy waiting

                except Exception as e:
                    print(f"Error in monitoring loop: {e}")
                    break

        monitor_thread = threading.Thread(target=monitoring_loop, daemon=True)
        monitor_thread.start()
        print("Real-time monitoring enabled")

    def create_log_filter(self, component: str = "", level: str = "", min_importance: str = ""):
        """
        Create a log filter function for real-time monitoring.

        Args:
            component: Component to filter by
            level: Log level to filter by
            min_importance: Minimum importance level

        Returns:
            Filter function for use with monitoring
        """
        def filter_function(messages):
            filtered = messages
            if component:
                filtered = [msg for msg in filtered if msg['component'] == component]
            if level:
                filtered = [msg for msg in filtered if msg['level'] == level.upper()]
            return filtered

        return filter_function


# Convenience functions for easy usage
def get_logger(application_name: str = "PythonApp", process_name: str = "") -> ASFMLoggerPython:
    """Get a configured ASFMLogger instance"""
    return ASFMLoggerPython(application_name, process_name)


def quick_log(message: str, level: str = "INFO", component: str = "Python"):
    """Quick logging function for simple use cases"""
    logger = get_logger()
    if level.upper() == "TRACE":
        logger.trace(message, component)
    elif level.upper() == "DEBUG":
        logger.debug(message, component)
    elif level.upper() == "INFO":
        logger.info(message, component)
    elif level.upper() == "WARN":
        logger.warn(message, component)
    elif level.upper() == "ERROR":
        logger.error(message, component)
    elif level.upper() == "CRITICAL":
        logger.critical(message, component)


def configure_global_logger(
    application_name: str,
    enable_database: bool = False,
    database_connection: str = "",
    log_file: str = "python_app.log"
):
    """Configure global logging for the application"""
    global _global_logger
    _global_logger = ASFMLoggerPython(application_name)
    _global_logger.configure_enhanced(
        enable_database=enable_database,
        database_connection=database_connection,
        log_file=log_file
    )
    return _global_logger


# Global logger instance
_global_logger = None


def get_global_logger() -> Optional[ASFMLoggerPython]:
    """Get the global logger instance"""
    return _global_logger