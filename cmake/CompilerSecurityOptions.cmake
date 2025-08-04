option(
  ENABLE_RECOMMENDED_SECURITY_FLAGS
  "Compile all targets with recommended security hardening flags. Check \
https://best.openssf.org/Compiler-Hardening-Guides/Compiler-Options-Hardening-Guide-for-C-and-C++ \
for information"
  OFF
)

set(
  COMPILER_SECURE_FLAGS_ENABLED
  -Wconversion
  -Wtrampolines
  -Wimplicit-fallthrough
  -U_FORTIFY_SOURCE
  -D_FORTIFY_SOURCE=3
  -D_GLIBCXX_ASSERTIONS
  -fstrict-flex-arrays=3
  -fstack-clash-protection
  -fstack-protector-strong
  -Wl,-z,nodlopen
  -Wl,-z,noexecstack
  -Wl,-z,relro
  -Wl,-z,now
  -fcf-protection=full
)
set(COMPILER_SECURE_FLAGS_DISABLED "")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13.0")
    list(APPEND COMPILER_SECURE_FLAGS_DISABLED -fstrict-flex-arrays=3)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12.0")
      list(APPEND COMPILER_SECURE_FLAGS_DISABLED -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3)
      if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.0")
        message(WARNING "GNU compiler version less than 8.0. Security flags are not supported")
      endif() # Version 8.0
    endif() # Version 12.0
  endif() # Version 13.0
elseif(CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?[Cc]lang")
  list(APPEND COMPILER_SECURE_FLAGS_DISABLED -Wl,-z,nodlopen -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now)
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "16.0")
    list(APPEND COMPILER_SECURE_FLAGS_DISABLED -fstrict-flex-arrays=3)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12.0")
      list(APPEND COMPILER_SECURE_FLAGS_DISABLED -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3)
      if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "11.0")
        message(WARNING "Clang compiler version less than 11.0. Security flags are not supported")
      endif() # Version 11.0
    endif() # Version 12.0
  endif() # Version 16.0
else()
  message(WARNING "Compiler ${CMAKE_CXX_COMPILER_ID} not supported for hardening flags")
  set(COMPILER_SECURE_FLAGS_ENABLED "")
endif()

list(REMOVE_ITEM COMPILER_SECURE_FLAGS_ENABLED ${COMPILER_SECURE_FLAGS_DISABLED})

# Enable flags
message(STATUS "Supported hardening flags: ${COMPILER_SECURE_FLAGS_ENABLED}")
if(NOT "${COMPILER_SECURE_FLAGS_DISABLED}" STREQUAL "")
  message(WARNING "Not supported hardening flags: ${COMPILER_SECURE_FLAGS_DISABLED}")
endif()

if(ENABLE_RECOMMENDED_SECURITY_FLAGS)
  add_compile_options(${COMPILER_SECURE_FLAGS_ENABLED})
endif()

macro(enable_security_flags_for_target name)
  message(STATUS "Enable hardening flags for target ${name}")
  target_compile_options(${name} PRIVATE ${COMPILER_SECURE_FLAGS_ENABLED})
endmacro()
