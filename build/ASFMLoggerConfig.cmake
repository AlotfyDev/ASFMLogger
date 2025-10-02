
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was ASFMLoggerConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

include("${CMAKE_CURRENT_LIST_DIR}/ASFMLoggerTargets.cmake")

check_required_components(ASFMLogger)

# Set up import targets
if(NOT TARGET ASFMLogger::ASFMLogger)
    add_library(ASFMLogger::ASFMLogger INTERFACE IMPORTED)
    set_target_properties(ASFMLogger::ASFMLogger PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${PACKAGE_PREFIX_DIR}/include"
        INTERFACE_LINK_LIBRARIES "Threads::Threads"
    )

    if()
        set_target_properties(ASFMLogger::ASFMLogger PROPERTIES
            INTERFACE_LINK_LIBRARIES "ASFMLogger::ASFMLogger;spdlog"
            INTERFACE_COMPILE_DEFINITIONS "ASFMLOGGER_SPDLOG_ENABLED"
        )
    endif()

    if(TRUE)
        set_target_properties(ASFMLogger::ASFMLogger PROPERTIES
            INTERFACE_LINK_LIBRARIES "ASFMLogger::ASFMLogger;odbc32;odbccp32"
            INTERFACE_COMPILE_DEFINITIONS "ASFMLOGGER_SQL_SERVER_SUPPORT"
        )
    endif()
endif()

# Package variables
set(ASFMLogger_VERSION "1.0.0")
set(ASFMLogger_INCLUDE_DIRS "${PACKAGE_PREFIX_DIR}/include")
set(ASFMLogger_LIBRARIES "ASFMLogger::ASFMLogger")
set(ASFMLogger_SPDLOG_ENABLED )
set(ASFMLogger_SQL_SERVER_SUPPORT TRUE)

# Multi-language wrapper locations
set(ASFMLogger_PYTHON_WRAPPER "${PACKAGE_PREFIX_DIR}/wrappers/python/asfm_logger.py")
set(ASFMLogger_CSHARP_WRAPPER "${PACKAGE_PREFIX_DIR}/wrappers/csharp")
set(ASFMLogger_MQL5_WRAPPER "${PACKAGE_PREFIX_DIR}/wrappers/mql5")

# Feature flags
set(ASFMLogger_FEATURES "")
if(ASFMLogger_SPDLOG_ENABLED)
    list(APPEND ASFMLogger_FEATURES "spdlog")
endif()
if(ASFMLogger_SQL_SERVER_SUPPORT)
    list(APPEND ASFMLogger_FEATURES "sql_server")
endif()
list(APPEND ASFMLogger_FEATURES "enhanced_logging" "multi_instance" "importance_framework")
set(ASFMLogger_FEATURES "${ASFMLogger_FEATURES}")
