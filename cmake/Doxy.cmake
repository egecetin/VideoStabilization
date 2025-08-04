# From https://stackoverflow.com/questions/60955881/cmake-for-doxygen

find_package(Doxygen)
if(DOXYGEN_FOUND)
  # set input and output files
  set(DOXYGEN_IN ${PROJECT_SOURCE_DIR}/doc/Doxyfile.in)
  set(DOXYGEN_OUT ${PROJECT_SOURCE_DIR}/Doxyfile)

  # request to configure the file
  configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

  # note the option ALL which allows to build the docs together with the application
  add_custom_target(
    docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
  )

  # Add this to your CMakeLists.txt after the doxygen target
  add_custom_command(
    TARGET docs
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/doc/html/doc
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/doc/logo.png ${PROJECT_SOURCE_DIR}/doc/html/doc
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/doc/GrafanaDashboard.png ${PROJECT_SOURCE_DIR}/doc/html/doc
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/doc/dependency-tree.svg ${PROJECT_SOURCE_DIR}/doc/html/doc
    COMMENT "Organizing HTML extra files into subfolders"
  )
else()
  message(WARNING "Doxygen need to be installed to generate the doxygen documentation!")
endif()
