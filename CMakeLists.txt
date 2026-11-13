cmake_minimum_required(VERSION 3.22)
project(programmor_test_device_native C)

enable_testing()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CORELIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../corelib" CACHE PATH "Pinned Corelib checkout")
set(TABLE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../table" CACHE PATH "Pinned Table checkout")

foreach(required
    "${CORELIB_DIR}/src/vendor/nanopb/pb.h"
    "${TABLE_DIR}/src/Table.h"
    "${TABLE_DIR}/extras/nanopb/table/nanopb.c")
  if(NOT EXISTS "${required}")
    message(FATAL_ERROR "Missing pinned dependency file: ${required}")
  endif()
endforeach()

add_library(test_device_model STATIC
  firmware/src/test_device.c
  firmware/src/generated/common.pb.c
  firmware/src/generated/test.pb.c
  "${TABLE_DIR}/src/table/table.c"
  "${TABLE_DIR}/extras/nanopb/table/nanopb.c"
  "${TABLE_DIR}/extras/nanopb/table/v1/table.pb.c"
  "${CORELIB_DIR}/src/vendor/nanopb/pb_common.c"
  "${CORELIB_DIR}/src/vendor/nanopb/pb_decode.c"
  "${CORELIB_DIR}/src/vendor/nanopb/pb_encode.c")
target_include_directories(test_device_model PUBLIC
  firmware/include
  firmware/src
  "${CORELIB_DIR}/src"
  "${CORELIB_DIR}/src/vendor/nanopb"
  "${TABLE_DIR}/src"
  "${TABLE_DIR}/extras/nanopb")
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(test_device_model PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
target_link_libraries(test_device_model PUBLIC m)

add_executable(test_device_tests tests/native/test_device_test.c)
target_link_libraries(test_device_tests PRIVATE test_device_model)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(test_device_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_test(NAME test_device_tests COMMAND test_device_tests)
