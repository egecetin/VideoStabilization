if(VideoStabilization_ENABLE_MEMLEAK_CHECK)
  include_directories(MemPlumber)
  add_subdirectory(MemPlumber)

  # Disable MemPlumber tests from parent
  add_custom_target(
    ExcludeMemPlumberTests
    ALL
    COMMAND rm -f "${PROJECT_BINARY_DIR}/tests/MemPlumber/CTestTestfile.cmake"
  )
endif()

configure_file(test-static-definitions.h.in test-static-definitions.h)

option(VideoStabilization_ENABLE_ASAN "Enables Address Sanitizer for VideoStabilization" OFF)
option(VideoStabilization_ENABLE_MSAN "Enables Memory Sanitizer for VideoStabilization" OFF)
option(VideoStabilization_ENABLE_USAN "Enables Undefined Behaviour Sanitizer for VideoStabilization" OFF)
option(VideoStabilization_ENABLE_TSAN "Enables Thread Sanitizer for VideoStabilization" OFF)

# Sanitizers
if(VideoStabilization_ENABLE_ASAN)
  message(STATUS "Enabling Address Sanitizer")
  target_compile_options("${PROJECT_NAME}-lib" PUBLIC "-fsanitize=address")
  target_link_options("${PROJECT_NAME}-lib" PUBLIC "-fsanitize=address")
endif()

if(VideoStabilization_ENABLE_MSAN)
  message(STATUS "Enabling Memory Sanitizer")
  target_compile_options("${PROJECT_NAME}-lib" PUBLIC "-fsanitize=memory")
  target_link_options("${PROJECT_NAME}-lib" PUBLIC "-fsanitize=memory")
endif()

if(VideoStabilization_ENABLE_USAN)
  message(STATUS "Enabling Undefined Behaviour Sanitizer")
  target_compile_options("${PROJECT_NAME}-lib" PUBLIC "-fsanitize=undefined")
  target_link_options("${PROJECT_NAME}-lib" PUBLIC "-fsanitize=undefined")
endif()

if(VideoStabilization_ENABLE_TSAN)
  message(STATUS "Enabling Thread Sanitizer")
  target_compile_options("${PROJECT_NAME}-lib" PUBLIC "-fsanitize=thread")
  target_link_options("${PROJECT_NAME}-lib" PUBLIC "-fsanitize=thread")
endif()

if(VideoStabilization_BUILD_UNITTESTS)
  add_subdirectory(gtest)
  add_subdirectory(unittests)
  list(APPEND TestExecutablesLocal UnitTestMain)
endif()

if(VideoStabilization_BUILD_FUZZTESTS)
  add_subdirectory(fuzztests)
  list(APPEND TestExecutablesLocal ${FuzzTestExecutables})
endif()

set(TestExecutables ${TestExecutablesLocal} PARENT_SCOPE)
