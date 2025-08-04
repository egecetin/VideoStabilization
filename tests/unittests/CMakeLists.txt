# Add sources
file(
  GLOB ProjectUnitTestSources
  Connection_UnitTests.cpp
  Logger_UnitTests.cpp
  Metrics_UnitTests.cpp
  Telnet_UnitTests.cpp
  Utils_UnitTests.cpp
  ZeroMQ_UnitTests.cpp
  gtest_main.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(UnitTestMain ${ProjectUnitTestSources})
target_include_directories(UnitTestMain PRIVATE "${PROJECT_BINARY_DIR}/tests")
target_link_libraries(UnitTestMain PRIVATE GTest::gtest_main "${PROJECT_NAME}-lib")

if(XXX_ENABLE_MEMLEAK_CHECK)
  target_link_libraries(UnitTestMain PRIVATE memplumber)
endif()

include(GoogleTest)
gtest_discover_tests(UnitTestMain)
