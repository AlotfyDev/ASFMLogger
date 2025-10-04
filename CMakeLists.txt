cmake_minimum_required(VERSION 3.20)
project(ASFMLogger VERSION 1.0.0
    DESCRIPTION "Advanced Scalable Fault-tolerant Multi-language Logger"
    LANGUAGES CXX C
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(ASFMLOGGER_HEADER_ONLY "Build header-only library" OFF)
option(ASFMLOGGER_BUILD_TESTS "Build unit tests" ON)
option(ASFMLOGGER_BUILD_BENCHMARKS "Build benchmark tests" ON)
option(ASFMLOGGER_USE_LOCAL_DEPS "Use local dependencies instead of vcpkg" OFF)

# Configure vcpkg if not using local deps
if(NOT ASFMLOGGER_USE_LOCAL_DEPS)
    include("${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Source files
set(CORE_SOURCES
    src/core/ASFMLogger.cpp
)

set(STATEFUL_SOURCES
    src/stateful/DatabaseLogger.cpp
    src/stateful/ImportanceMapper.cpp
    src/stateful/LoggerInstance.cpp
    src/stateful/LogMessage.cpp
    src/stateful/SmartMessageQueue.cpp
)

set(ENHANCED_SOURCES
    src/enhanced/ASFMLoggerEnhanced.cpp
)

# Define libraries
if(ASFMLOGGER_HEADER_ONLY)
    # Header-only library
    add_library(ASFMLogger INTERFACE)
    target_include_directories(ASFMLogger INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
else()
    # Static and shared libraries
    add_library(ASFMLogger ${CORE_SOURCES})
    target_include_directories(ASFMLogger PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )

    add_library(ASFMLoggerEnhanced ${CORE_SOURCES} ${STATEFUL_SOURCES} ${ENHANCED_SOURCES})
    target_include_directories(ASFMLoggerEnhanced PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )

    # Find dependencies
    find_package(nlohmann_json CONFIG REQUIRED)
    find_package(spdlog CONFIG REQUIRED)
    find_package(fmt CONFIG REQUIRED)

    target_link_libraries(ASFMLogger PUBLIC
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        fmt::fmt
    )

    target_link_libraries(ASFMLoggerEnhanced PRIVATE
        ASFMLogger
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        fmt::fmt
    )
endif()

# Tests
if(ASFMLOGGER_BUILD_TESTS)
    enable_testing()

    # Core unit tests
    set(CORE_TEST_SOURCES
        tests/unit/core/unit_test_logmessage.cpp
        tests/unit/core/unit_test_loggerinstance.cpp
        tests/unit/core/unit_test_importance_mapper.cpp
        tests/unit/core/unit_test_database_logger.cpp
        tests/unit/core/unit_test_smart_message_queue.cpp
    )

    # Integration tests
    set(INTEGRATION_TEST_SOURCES
        tests/integration/simple_stateful_test.cpp
        tests/integration/integration_test.cpp
        tests/integration/test_integration_testing.cpp
        tests/integration/test_stateful_layer.cpp
    )

    # Test executables
    add_executable(ASFMLoggerTests ${CORE_TEST_SOURCES} tests/test_main.cpp)
    add_executable(ASFMLoggerIntegrationTests ${CORE_TEST_SOURCES} ${INTEGRATION_TEST_SOURCES} tests/test_main.cpp)

    # Link tests
    target_link_libraries(ASFMLoggerTests PRIVATE ASFMLoggerEnhanced GTest::gtest_main)
    target_link_libraries(ASFMLoggerIntegrationTests PRIVATE ASFMLoggerEnhanced GTest::gtest_main)

    # Register tests
    add_test(NAME CoreTests COMMAND ASFMLoggerTests)
    add_test(NAME IntegrationTests COMMAND ASFMLoggerIntegrationTests)
endif()

# Benchmarks
if(ASFMLOGGER_BUILD_BENCHMARKS)
    find_package(benchmark CONFIG QUIET)
    if(benchmark_FOUND)
        set(BENCHMARK_SOURCES
            tests/benchmarks/benchmark_main.cpp
            tests/benchmarks/benchmark_logmessage.cpp
            tests/benchmarks/benchmark_loggerinstance.cpp
            tests/benchmarks/benchmark_importance_mapper.cpp
            tests/benchmarks/benchmark_smart_message_queue.cpp
            tests/benchmarks/benchmark_database_logger.cpp
            tests/benchmarks/benchmark_infrastructure.cpp
        )

        add_executable(ASFMLoggerBenchmarks ${BENCHMARK_SOURCES})
        target_link_libraries(ASFMLoggerBenchmarks PRIVATE ASFMLoggerEnhanced benchmark::benchmark_main)
    else()
        message(WARNING "Google Benchmark not found - benchmarks disabled")
    endif()
endif()

# Examples
set(EXAMPLE_SOURCES
    examples/built_library_example.cpp
    examples/simple_working_example.cpp
    examples/header_only_example.cpp
    examples/test_header_only.cpp
)

foreach(example_file ${EXAMPLE_SOURCES})
    get_filename_component(example_name ${example_file} NAME_WE)
    add_executable(${example_name} ${example_file})
    target_link_libraries(${example_name} PRIVATE ASFMLoggerEnhanced)
endforeach()

# Install targets
install(TARGETS ASFMLogger ASFMLoggerEnhanced
    EXPORT ASFMLoggerTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include/)

# Export
install(EXPORT ASFMLoggerTargets
    FILE ASFMLoggerTargets.cmake
    NAMESPACE ASFMLogger::
    DESTINATION lib/cmake/ASFMLogger
)

# Package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "cmake/ASFMLoggerConfigVersion.cmake"
    VERSION ${ASFMLogger_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "cmake/ASFMLoggerConfig.cmake.in"
    "cmake/ASFMLoggerConfig.cmake"
    INSTALL_DESTINATION lib/cmake/ASFMLogger
    PATH_VARS
        CMAKE_INSTALL_INCLUDEDIR
)
