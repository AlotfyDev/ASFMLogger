# C# Wrapper CMakeLists.txt

# Find .NET CLI
find_program(DOTNET_EXECUTABLE dotnet)
if(NOT DOTNET_EXECUTABLE)
    message(WARNING ".NET CLI not found. C# wrapper will not be built.")
    return()
endif()

# C# project file
set(CSHARP_PROJECT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ASFMLogger.Wrapper.csproj")

# Create C# project file if it doesn't exist
if(NOT EXISTS "${CSHARP_PROJECT_FILE}")
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/ASFMLogger.Wrapper.csproj.in
        ${CSHARP_PROJECT_FILE}
        @ONLY
    )
endif()

# Build C# wrapper
add_custom_target(ASFMLoggerCSharpWrapper ALL
    COMMAND ${DOTNET_EXECUTABLE} build ${CSHARP_PROJECT_FILE} --configuration Release --output ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/csharp
    COMMENT "Building C# wrapper"
    DEPENDS ASFMLogger
)

# Install C# wrapper
install(DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/csharp/
    DESTINATION ${CMAKE_INSTALL_PREFIX}/wrappers/csharp
    FILES_MATCHING PATTERN "*.dll"
)

# Test C# integration
add_test(NAME CSharpWrapperBuildTest
    COMMAND ${DOTNET_EXECUTABLE} build ${CSHARP_PROJECT_FILE} --configuration Release
)

# Package creation
add_custom_target(csharp_package
    COMMAND ${DOTNET_EXECUTABLE} pack ${CSHARP_PROJECT_FILE} --configuration Release --output ${CMAKE_BINARY_DIR}/packages
    COMMENT "Creating C# NuGet package"
    DEPENDS ASFMLoggerCSharpWrapper
)