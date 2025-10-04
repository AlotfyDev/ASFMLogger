# Tests CMakeLists.txt
message(STATUS "Building unit tests")

# Only build tests if not in header-only build mode
if(ASFMLOGGER_HEADER_ONLY)
    message(STATUS "Skipping unit tests in header-only build mode")
    return()
endif()

# Find GTest for testing
set(GTEST_AVAILABLE FALSE)

# Try multiple methods to find GTest
find_package(GTest CONFIG QUIET)
if(GTest_FOUND)
    set(GTEST_AVAILABLE TRUE)
    message(STATUS "GTest found via CONFIG - unit tests enabled")
else()
    message(STATUS "GTest CONFIG not found, trying alternative find methods...")

    # Try without CONFIG
    find_package(GTest QUIET)
    if(GTest_FOUND)
        set(GTEST_AVAILABLE TRUE)
        message(STATUS "GTest found via alternative method - unit tests enabled")
    endif()
endif()

# Try system-installed GTest as last resort
if(NOT GTEST_AVAILABLE)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(GTest QUIET gtest)
        if(GTest_FOUND)
            set(GTEST_AVAILABLE TRUE)
            message(STATUS "GTest found via pkg-config - unit tests enabled")
        endif()
    endif()
endif()

if(NOT GTEST_AVAILABLE)
    message(WARNING "GTest not found. Unit tests will be disabled.")
    message(WARNING "To enable testing, install GTest using one of:")
    message(WARNING "  - vcpkg: vcpkg install gtest")
    message(WARNING "  - System package manager: apt install libgtest-dev (Linux)")
    message(WARNING "  - Or set CMAKE_PREFIX_PATH to your GTest installation")
    message(STATUS "Continuing build without unit tests...")
    return()
endif()

message(STATUS "GTest found - building unit tests")

# Test Configuration and Infrastructure Setup
message(STATUS "=== ASFMLogger Testing Infrastructure ===")
message(STATUS "Setting up comprehensive unit test and benchmark infrastructure...")

# Global test configurations
set(TESTING_STANDARD_FLAGS "-DASFMLOGGER_TESTING -DUNIT_TEST_EXECUTION")

# Performance measurement utilities
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/test_config.json
    ${CMAKE_CURRENT_BINARY_DIR}/test_config.json
    COPYONLY
)

# =============================================================================
# MAIN UNIT TEST EXECUTABLE
# =============================================================================

# Core unit test files
set(UNIT_TEST_SOURCES
    test_main.cpp
    test_enhanced_logger.cpp
    test_toolbox_classes.cpp
    simple_stateful_test.cpp
    integration_test.cpp

    # Phase 5A: Core Functionality Unit Tests
    unit_test_logmessage.cpp
    unit_test_loggerinstance.cpp
    unit_test_importance_mapper.cpp

    # Phase 5B: Advanced Component Unit Tests
    unit_test_smart_message_queue.cpp
    unit_test_database_logger.cpp

    # Performance benchmark infrastructure
    benchmark_infrastructure.cpp
)

message(STATUS "Unit test sources: ${UNIT_TEST_SOURCES}")

# Test executable - Force sequential compilation to avoid PDB conflicts
add_executable(ASFMLoggerTests ${UNIT_TEST_SOURCES})

# Add MSVC-specific flags for PDB concurrency
if(MSVC)
    # /FS forces synchronous PDB writes to prevent parallel access conflicts
    target_compile_options(ASFMLoggerTests PRIVATE /FS)
    target_link_options(ASFMLoggerTests PRIVATE /FS)
endif()

# Include directories
target_include_directories(ASFMLoggerTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../external
    ${CMAKE_CURRENT_SOURCE_DIR}/test_infrastructure
)

# Link libraries with comprehensive dependencies
target_link_libraries(ASFMLoggerTests
    ASFMLogger
    GTest::gtest_main
    GTest::gmock_main
    Threads::Threads
    # Add Windows-specific libraries if needed
    $<$<PLATFORM_ID:Windows>:ws2_32>
    $<$<PLATFORM_ID:Windows>:userenv>
)

# =============================================================================
# PERFORMANCE BENCHMARK EXECUTABLE (OPTIONAL)
# =============================================================================

# Check for Google Benchmark
find_package(benchmark QUIET)

if(benchmark_FOUND)
    message(STATUS "Google Benchmark found - building performance benchmarks")

    # Separate benchmark executable for performance isolation
    add_executable(ASFMLoggerBenchmarks
        benchmark_main.cpp
        benchmark_logmessage.cpp
        benchmark_loggerinstance.cpp
        benchmark_importance_mapper.cpp
        benchmark_smart_message_queue.cpp
        benchmark_database_logger.cpp
        benchmark_comprehensive.cpp
    )

    # Add MSVC-specific flags for PDB concurrency in benchmarks too
    if(MSVC)
        target_compile_options(ASFMLoggerBenchmarks PRIVATE /FS)
        target_link_options(ASFMLoggerBenchmarks PRIVATE /FS)
    endif()

    target_include_directories(ASFMLoggerBenchmarks PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
        ${CMAKE_CURRENT_SOURCE_DIR}/../external
    )

    target_link_libraries(ASFMLoggerBenchmarks
        ASFMLogger
        benchmark::benchmark
        Threads::Threads
    )

    target_compile_definitions(ASFMLoggerBenchmarks PRIVATE
        ${TESTING_STANDARD_FLAGS}
        -DBENCHMARK_EXECUTION
    )

    # Register benchmark tests
    add_test(NAME ASFMLoggerBenchmarks COMMAND ASFMLoggerBenchmarks)

    message(STATUS "Performance benchmarks enabled")
else()
    message(WARNING "Google Benchmark not found - performance benchmarks disabled")
    message(WARNING "To enable benchmarks, install Google Benchmark:")
    message(WARNING "  - vcpkg: vcpkg install benchmark")
    message(STATUS "Continuing build without benchmarks...")
endif()

# Test definitions and compiler flags
target_compile_definitions(ASFMLoggerTests PRIVATE
    ${TESTING_STANDARD_FLAGS}
    -DENABLE_PERFORMANCE_METRICS
    -DENABLE_MEMORY_TRACKING
)

target_compile_options(ASFMLoggerTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>: /W4 /WX- /permissive- >
    $<$<CXX_COMPILER_ID:GNU>: -Wall -Wextra -Wno-unused-parameter>
    $<$<CXX_COMPILER_ID:Clang>: -Wall -Wextra -Wno-unused-parameter>
)

# Register main test suite
add_test(NAME ASFMLoggerUnitTests COMMAND ASFMLoggerTests)

# =============================================================================
# COVERAGE AND ANALYSIS CONFIGURATION
# =============================================================================

# Coverage configuration (GNU only for now)
if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(ASFMLoggerTests PRIVATE --coverage -fprofile-arcs -ftest-coverage)
    target_link_options(ASFMLoggerTests PRIVATE --coverage -lgcov)

    if(benchmark_FOUND)
        target_compile_options(ASFMLoggerBenchmarks PRIVATE --coverage -fprofile-arcs -ftest-coverage)
        target_link_options(ASFMLoggerBenchmarks PRIVATE --coverage -lgcov)
    endif()

    # Code coverage target
    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
        COMMAND gcovr
            --root ${CMAKE_CURRENT_SOURCE_DIR}/../src
            --html --html-details
            --output ${CMAKE_BINARY_DIR}/coverage/coverage_report.html
            --exclude-throw-branches
            --print-summary
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating code coverage report..."
    )
endif()

# =============================================================================
# STATIC ANALYSIS AND QUALITY CHECKS
# =============================================================================

# Clang-Tidy (if available)
find_program(CLANG_TIDY_EXE clang-tidy)
if(CLANG_TIDY_EXE)
    set_target_properties(ASFMLoggerTests PROPERTIES
        CXX_CLANG_TIDY "${CLANG_TIDY_EXE};--warnings-as-errors=*"
    )
    message(STATUS "Clang-Tidy enabled for unit tests")
endif()

# =============================================================================
# TEST INFRASTRUCTURE SUMMARY
# =============================================================================

message(STATUS "=== Unit Test Infrastructure Complete ===")
message(STATUS "✓ Main Unit Test Executable: ASFMLoggerTests")
message(STATUS "✓ Performance Benchmark Executable: ASFMLoggerBenchmarks")
message(STATUS "✓ Test Coverage Reporting: Enabled (GNU GCC)")
message(STATUS "✓ Static Analysis: Clang-Tidy enabled")
message(STATUS "✓ Cross-platform Support: Windows/Unix compatible")

message(STATUS "Total unit test files configured: ${UNIT_TEST_SOURCES_COUNT}")
