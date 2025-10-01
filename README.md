# ASFMLogger

A high-performance, thread-safe C++ logging library built on top of [spdlog](https://github.com/gabime/spdlog). ASFMLogger (Abstract Shared File Map Logger) provides a singleton-based logging interface with advanced features for modern C++ applications.

## Features

- 🚀 **High Performance**: Built on the fast spdlog library
- 🧵 **Thread-Safe**: All operations are thread-safe with proper synchronization
- 🎯 **Singleton Pattern**: Single instance across your application
- 📁 **Multiple Outputs**: Console and rotating file logging
- 🔧 **Configurable**: Customizable log levels, file sizes, and rotation
- 💾 **Error Tracking**: Built-in last error storage and retrieval
- 🎨 **Colored Console Output**: Colored terminal output for better readability
- 📊 **Multiple Log Levels**: trace, debug, info, warn, error, critical

## Installation

### Prerequisites

- C++17 or higher
- [spdlog](https://github.com/gabime/spdlog) library
- CMake (recommended for building)

### Building

```bash
mkdir build
cd build
cmake ..
make
```

### Integration

Include the header in your project:

```cpp
#include "ASFMLogger.hpp"
```

## Usage

### Basic Usage

```cpp
#include "ASFMLogger.hpp"

// Get the logger instance
auto logger = Logger::getInstance();

// Configure the logger (optional)
logger->configure(
    true,                    // console_output
    "app.log",              // log_file_name
    10485760,               // max_file_size (10MB)
    5,                      // max_files
    spdlog::level::info     // log_level
);

// Log messages
logger->info("Application started");
logger->debug("Debug information: {}", some_variable);
logger->warn("This is a warning");
logger->error("An error occurred: {}", error_message);
logger->critical("Critical error!");
```

### Advanced Usage

```cpp
// Template-based logging with formatting
logger->info("User {} logged in at {}", username, login_time);

// Component-specific error logging
logger->LogError("Database", "Connect", "Connection failed");

// Get last error
std::string lastError = Logger::GetLastError();

// Clear errors
Logger::Clear();
```

### Log Levels

- `trace`: Most verbose, typically for development debugging
- `debug`: Debug information for development
- `info`: General information about application flow
- `warn`: Warning messages for potentially harmful situations
- `error`: Error events that might still allow the application to continue
- `critical`: Critical errors that may cause the application to terminate

## Configuration

### Default Configuration

If no configuration is provided, the logger defaults to:
- Console output enabled
- Log file: `framework.log`
- Max file size: 10MB
- Max files: 5
- Log level: `info`

### Custom Configuration

```cpp
logger->configure(
    false,                          // Disable console output
    "custom.log",                   // Custom log file name
    52428800,                       // 50MB max file size
    10,                             // Keep 10 rotated files
    spdlog::level::debug            // Debug level logging
);
```

### Log Pattern

The default log pattern is:
```
[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [%t] %v
```

Which produces output like:
```
[2023-12-07 14:30:15.123] [framework] [info] [main] Application started
```

## Dependencies

- **[spdlog](https://github.com/gabime/spdlog)**: Fast C++ logging library
- **C++17** or higher
- **Threading support** (std::mutex, std::thread)

## Thread Safety

ASFMLogger is fully thread-safe:
- Singleton instance creation is thread-safe
- All logging operations are atomic
- Error storage uses mutex protection
- Multiple threads can log simultaneously without data races

## Error Handling

The logger provides built-in error tracking:

```cpp
// Log an error
logger->error("Database connection failed");

// Retrieve the last error
std::string lastError = Logger::GetLastError();
std::cout << "Last error: " << lastError << std::endl;

// Clear all stored errors
Logger::Clear();
```

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## Support

For questions, issues, or feature requests, please open an issue on GitHub.

---

**Made with ❤️ for high-performance C++ applications**