# CMake generated Testfile for 
# Source directory: D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/wrappers/csharp
# Build directory: D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/build/wrappers/csharp
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test([=[CSharpWrapperBuildTest]=] "C:/Program Files/dotnet/dotnet.exe" "build" "D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/wrappers/csharp/ASFMLogger.Wrapper.csproj" "--configuration" "Release")
  set_tests_properties([=[CSharpWrapperBuildTest]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/wrappers/csharp/CMakeLists.txt;36;add_test;D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/wrappers/csharp/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test([=[CSharpWrapperBuildTest]=] "C:/Program Files/dotnet/dotnet.exe" "build" "D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/wrappers/csharp/ASFMLogger.Wrapper.csproj" "--configuration" "Release")
  set_tests_properties([=[CSharpWrapperBuildTest]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/wrappers/csharp/CMakeLists.txt;36;add_test;D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/wrappers/csharp/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test([=[CSharpWrapperBuildTest]=] "C:/Program Files/dotnet/dotnet.exe" "build" "D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/wrappers/csharp/ASFMLogger.Wrapper.csproj" "--configuration" "Release")
  set_tests_properties([=[CSharpWrapperBuildTest]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/wrappers/csharp/CMakeLists.txt;36;add_test;D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/wrappers/csharp/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test([=[CSharpWrapperBuildTest]=] "C:/Program Files/dotnet/dotnet.exe" "build" "D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/wrappers/csharp/ASFMLogger.Wrapper.csproj" "--configuration" "Release")
  set_tests_properties([=[CSharpWrapperBuildTest]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/wrappers/csharp/CMakeLists.txt;36;add_test;D:/CppBridgeVSC/AbstractSharedFileMap/ASFMLogger/wrappers/csharp/CMakeLists.txt;0;")
else()
  add_test([=[CSharpWrapperBuildTest]=] NOT_AVAILABLE)
endif()
