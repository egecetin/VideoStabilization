# Check compiler
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  message(SEND_ERROR "Fuzz testing not supported for ${CMAKE_CXX_COMPILER_ID} compiler")
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_BINARY_DIR}/tests)

# Fuzz sources
set(
  ProjectFuzzTestSources
  "\
Hasher_FuzzTests;\
Http_FuzzTests;\
Telnet_FuzzTests;"
)

# Create fuzz targets
foreach(SourceEntry ${ProjectFuzzTestSources})
  set(TARGET_NAME ${SourceEntry}.Fuzzer)

  add_executable(${TARGET_NAME} ${SourceEntry}.cpp)
  target_link_libraries(${TARGET_NAME} PRIVATE "${PROJECT_NAME}-lib")
  target_compile_options(${TARGET_NAME} PRIVATE "-fsanitize=fuzzer")
  target_link_options(${TARGET_NAME} PRIVATE "-fsanitize=fuzzer")

  add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME} -runs=10000)
  list(APPEND FuzzTestExecutablesLocal ${TARGET_NAME})
endforeach()

set(FuzzTestExecutables ${FuzzTestExecutablesLocal} PARENT_SCOPE)
