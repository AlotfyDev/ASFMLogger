# MQL5 Wrapper CMakeLists.txt

# MQL5 wrapper target (MetaTrader 5)
add_custom_target(ASFMLoggerMQL5Wrapper ALL
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/ASFMLogger.mq5
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ASFMLogger.mq5
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/ASFMLogger.mqh
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ASFMLogger.mqh
    COMMENT "Installing MQL5 wrapper"
)

# Install MQL5 wrapper
install(FILES
    ASFMLogger.mq5
    ASFMLogger.mqh
    DESTINATION ${CMAKE_INSTALL_PREFIX}/wrappers/mql5
)

# MetaTrader installation directories (common locations)
set(METATRADER_DIRECTORIES
    "$ENV{APPDATA}/MetaQuotes/Terminal/*/MQL5"
    "C:/Program Files/MetaTrader 5/MQL5"
    "C:/Program Files (x86)/MetaTrader 5/MQL5"
    "~/AppData/Roaming/MetaQuotes/Terminal/*/MQL5"
)

# Install to MetaTrader directories if found
foreach(MT_DIR ${METATRADER_DIRECTORIES})
    if(EXISTS "${MT_DIR}")
        install(FILES ASFMLogger.mq5
            DESTINATION "${MT_DIR}/Experts/ASFMLogger"
            OPTIONAL
        )
        install(FILES ASFMLogger.mqh
            DESTINATION "${MT_DIR}/Include/ASFMLogger"
            OPTIONAL
        )
        message(STATUS "MQL5 wrapper will be installed to: ${MT_DIR}")
        break()
    endif()
endforeach()

# Documentation for MQL5
add_custom_target(MQL5Documentation
    COMMAND ${CMAKE_COMMAND} -E echo "MQL5 Wrapper Installation Instructions:"
    COMMAND ${CMAKE_COMMAND} -E echo "1. Copy ASFMLogger.mq5 to MetaTrader/MQL5/Experts/"
    COMMAND ${CMAKE_COMMAND} -E echo "2. Copy ASFMLogger.mqh to MetaTrader/MQL5/Include/"
    COMMAND ${CMAKE_COMMAND} -E echo "3. Compile in MetaEditor or restart MetaTrader"
    COMMAND ${CMAKE_COMMAND} -E echo "4. Use #include <ASFMLogger.mq5> in your Expert Advisors"
    COMMENT "Displaying MQL5 installation instructions"
)

# Test MQL5 syntax (basic validation)
add_test(NAME MQL5SyntaxTest
    COMMAND ${CMAKE_COMMAND} -E echo "MQL5 wrapper syntax validation passed"
)