# CMake generated Testfile for 
# Source directory: D:/Audio/rtaudio-5.1.0/tests
# Build directory: D:/Audio/rtaudio-5.1.0/windowsBuild/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test("apinames" "D:/Audio/rtaudio-5.1.0/windowsBuild/tests/Debug/apinames.exe")
  set_tests_properties("apinames" PROPERTIES  _BACKTRACE_TRIPLES "D:/Audio/rtaudio-5.1.0/tests/CMakeLists.txt;32;add_test;D:/Audio/rtaudio-5.1.0/tests/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test("apinames" "D:/Audio/rtaudio-5.1.0/windowsBuild/tests/Release/apinames.exe")
  set_tests_properties("apinames" PROPERTIES  _BACKTRACE_TRIPLES "D:/Audio/rtaudio-5.1.0/tests/CMakeLists.txt;32;add_test;D:/Audio/rtaudio-5.1.0/tests/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test("apinames" "D:/Audio/rtaudio-5.1.0/windowsBuild/tests/MinSizeRel/apinames.exe")
  set_tests_properties("apinames" PROPERTIES  _BACKTRACE_TRIPLES "D:/Audio/rtaudio-5.1.0/tests/CMakeLists.txt;32;add_test;D:/Audio/rtaudio-5.1.0/tests/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test("apinames" "D:/Audio/rtaudio-5.1.0/windowsBuild/tests/RelWithDebInfo/apinames.exe")
  set_tests_properties("apinames" PROPERTIES  _BACKTRACE_TRIPLES "D:/Audio/rtaudio-5.1.0/tests/CMakeLists.txt;32;add_test;D:/Audio/rtaudio-5.1.0/tests/CMakeLists.txt;0;")
else()
  add_test("apinames" NOT_AVAILABLE)
endif()
