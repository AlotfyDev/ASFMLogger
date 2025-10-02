# Tests CMakeLists.txt
find_package(GTest CONFIG REQUIRED)

message(STATUS "GTest found - building unit tests")

# Test executable
add_executable(ASFMLoggerTests
    test_main.cpp
    test_log_data_structures.cpp
    test_logger_instance.cpp
    test_importance_framework.cpp
    test_persistence_policies.cpp
    test_smart_queues.cpp
    test_database_logger.cpp
    test_configuration_manager.cpp
    test_monitoring_manager.cpp
    test_enhanced_logger.cpp
    test_toolbox_classes.cpp
    test_thread_safety.cpp
    test_performance.cpp
)

# Include directories
target_include_directories(ASFMLoggerTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

# Link libraries
target_link_libraries(ASFMLoggerTests
    ASFMLogger
    GTest::gtest_main
    GTest::gmock_main
    Threads::Threads
)

# Test definitions
target_compile_definitions(ASFMLoggerTests PRIVATE
    -DASFMLOGGER_TESTING
)

# Register tests
add_test(NAME ASFMLoggerUnitTests COMMAND ASFMLoggerTests)

# Coverage configuration (if available)
if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(ASFMLoggerTests PRIVATE --coverage)
    target_link_options(ASFMLoggerTests PRIVATE --coverage)
endif()

# Test discovery
gtest_discover_tests(ASFMLoggerTests)

message(STATUS "Unit tests configured successfully")