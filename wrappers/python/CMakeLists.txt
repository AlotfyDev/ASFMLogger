# Python Wrapper CMakeLists.txt

# Find Python
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# Python wrapper target
add_custom_target(ASFMLoggerPythonWrapper ALL
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/asfm_logger.py
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/asfm_logger.py
    COMMENT "Installing Python wrapper"
)

# Install Python wrapper
install(FILES asfm_logger.py
    DESTINATION ${CMAKE_INSTALL_PREFIX}/wrappers/python
)

# Python setup script
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in
    ${CMAKE_CURRENT_BINARY_DIR}/setup.py
    @ONLY
)

# Python package installation (optional)
add_custom_target(install_python_package
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/setup.py install
    COMMENT "Installing ASFMLogger Python package"
    DEPENDS ASFMLoggerPythonWrapper
)

# Test Python integration
add_test(NAME PythonWrapperImportTest
    COMMAND ${Python3_EXECUTABLE} -c "import sys; sys.path.append('${CMAKE_RUNTIME_OUTPUT_DIRECTORY}'); import asfm_logger; print('Python wrapper import successful')"
)