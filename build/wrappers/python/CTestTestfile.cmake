# CMake generated Testfile for 
# Source directory: D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/wrappers/python
# Build directory: D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/build/wrappers/python
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test([=[PythonWrapperImportTest]=] "C:/ProgramData/anaconda3/python.exe" "-c" "import sys; sys.path.append('D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/build/bin'); import asfm_logger; print('Python wrapper import successful')")
  set_tests_properties([=[PythonWrapperImportTest]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/wrappers/python/CMakeLists.txt;34;add_test;D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/wrappers/python/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test([=[PythonWrapperImportTest]=] "C:/ProgramData/anaconda3/python.exe" "-c" "import sys; sys.path.append('D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/build/bin'); import asfm_logger; print('Python wrapper import successful')")
  set_tests_properties([=[PythonWrapperImportTest]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/wrappers/python/CMakeLists.txt;34;add_test;D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/wrappers/python/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test([=[PythonWrapperImportTest]=] "C:/ProgramData/anaconda3/python.exe" "-c" "import sys; sys.path.append('D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/build/bin'); import asfm_logger; print('Python wrapper import successful')")
  set_tests_properties([=[PythonWrapperImportTest]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/wrappers/python/CMakeLists.txt;34;add_test;D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/wrappers/python/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test([=[PythonWrapperImportTest]=] "C:/ProgramData/anaconda3/python.exe" "-c" "import sys; sys.path.append('D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/build/bin'); import asfm_logger; print('Python wrapper import successful')")
  set_tests_properties([=[PythonWrapperImportTest]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/wrappers/python/CMakeLists.txt;34;add_test;D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/wrappers/python/CMakeLists.txt;0;")
else()
  add_test([=[PythonWrapperImportTest]=] NOT_AVAILABLE)
endif()
