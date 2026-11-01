cmake_minimum_required(VERSION 3.20)
project(numworkscc LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PICOC_SOURCES
  third_party/picoc/table.c
  third_party/picoc/lex.c
  third_party/picoc/parse.c
  third_party/picoc/expression.c
  third_party/picoc/heap.c
  third_party/picoc/type.c
  third_party/picoc/variable.c
  third_party/picoc/clibrary.c
  third_party/picoc/platform.c
  third_party/picoc/include.c
  third_party/picoc/debug.c
  third_party/picoc/cstdlib/stdio.c
  third_party/picoc/cstdlib/math.c
  third_party/picoc/cstdlib/string.c
  third_party/picoc/cstdlib/stdlib.c
  third_party/picoc/cstdlib/time.c
  third_party/picoc/cstdlib/errno.c
  third_party/picoc/cstdlib/ctype.c
  third_party/picoc/cstdlib/stdbool.c
  third_party/picoc/cstdlib/unistd.c
  third_party/picoc/platform_nwcc.c
)

add_library(numworkscc_core
  src/compiler.cpp
  src/runtime.cpp
  ${PICOC_SOURCES}
)
target_include_directories(numworkscc_core PUBLIC include)
target_compile_options(numworkscc_core PRIVATE -Wall -Wextra -Wpedantic)
target_compile_definitions(numworkscc_core PRIVATE UNIX_HOST)
set_source_files_properties(${PICOC_SOURCES} PROPERTIES LANGUAGE C)
set_source_files_properties(${PICOC_SOURCES} PROPERTIES COMPILE_DEFINITIONS UNIX_HOST)

add_executable(numworkscc src/main.cpp)
target_link_libraries(numworkscc PRIVATE numworkscc_core)

add_library(numworkscc_epsilon_adapter STATIC platform/epsilon/epsilon_adapter.cpp)
target_include_directories(numworkscc_epsilon_adapter PUBLIC include platform)
target_link_libraries(numworkscc_epsilon_adapter PUBLIC numworkscc_core)

add_library(numworkscc_upsilon_adapter STATIC platform/upsilon/upsilon_adapter.cpp)
target_include_directories(numworkscc_upsilon_adapter PUBLIC include platform)
target_link_libraries(numworkscc_upsilon_adapter PUBLIC numworkscc_core)

enable_testing()
add_executable(numworkscc_tests tests/core_tests.cpp)
target_link_libraries(numworkscc_tests PRIVATE numworkscc_core)
add_test(NAME core_tests COMMAND numworkscc_tests)
