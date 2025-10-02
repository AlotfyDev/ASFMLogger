# Examples CMakeLists.txt

# Header-only example
add_executable(header_only_example header_only_example.cpp)
target_compile_definitions(header_only_example PRIVATE ASFMLOGGER_HEADER_ONLY)
target_include_directories(header_only_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Built library example (if library is available)
if(NOT ASFMLOGGER_HEADER_ONLY)
    add_executable(built_library_example built_library_example.cpp)
    target_link_libraries(built_library_example ASFMLogger)
    target_include_directories(built_library_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
endif()

# Install examples
install(TARGETS header_only_example
    RUNTIME DESTINATION examples
)

if(NOT ASFMLOGGER_HEADER_ONLY)
    install(TARGETS built_library_example
        RUNTIME DESTINATION examples
    )
endif()

install(FILES
    header_only_example.cpp
    built_library_example.cpp
    DESTINATION examples
)