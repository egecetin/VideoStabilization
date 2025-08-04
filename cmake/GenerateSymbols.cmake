macro(generate_symbols_for_target target)
  message(STATUS "Enabling symbol file generation for ${target}")
  add_custom_command(
    TARGET ${target}
    POST_BUILD
    COMMAND
      ${PROJECT_SOURCE_DIR}/scripts/dump_syms.py --dump-syms
      ${CMAKE_BINARY_DIR}/breakpad_bin/src/tools/linux/dump_syms/dump_syms --binary-dir $<TARGET_FILE:${target}>
      --output-dir ${CMAKE_BINARY_DIR}/syms
    COMMENT "Creating symbols ${target}"
  )
  add_dependencies(${target} breakpad)
endmacro()
