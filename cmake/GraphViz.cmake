#
# Copyright (C) 2019 by George Cave - gcave@stablecoder.ca
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
# specific language governing permissions and limitations under the License.

find_program(DOT_EXE "dot")

if(DOT_EXE)
  message(STATUS "dot found: ${DOT_EXE}")
else()
  message(STATUS "dot not found!")
endif()

set(DOT_OUTPUT_TYPE "svg" CACHE STRING "Build a dependency graph. Options are dot output types: ps, png, pdf...")

if(DOT_EXE)
  add_custom_target(
    dependency-graph
    COMMAND ${CMAKE_COMMAND} ${PROJECT_SOURCE_DIR} --graphviz=${PROJECT_BINARY_DIR}/graphviz/${PROJECT_NAME}.dot
    COMMAND
      ${DOT_EXE} -T${DOT_OUTPUT_TYPE} ${PROJECT_BINARY_DIR}/graphviz/${PROJECT_NAME}.dot -o
      ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-tree.${DOT_OUTPUT_TYPE}
  )

  add_custom_command(
    TARGET dependency-graph
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E echo
      "Dependency graph generated and located at ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-tree.${DOT_OUTPUT_TYPE}" ;
  )
endif()
