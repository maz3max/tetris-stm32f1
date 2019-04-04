cmake_minimum_required(VERSION 3.0)
set(CMAKE_TOOLCHAIN_FILE cmake/toolchain-arm-none-eabi.cmake)
project(libopencm3_freertos_bluepill_blink)
include(cmake/blue-pill.cmake)
include(cmake/libopencm3.cmake)
include(cmake/freertos.cmake)
include(cmake/freertos-libopencm3-glue.cmake)
include(cmake/st-link.cmake)

# Targets to build

## Blink in C language
add_executable(
  blink_c
  blink.c
  ${FREERTOS_OBJECTS}
  ${FREERTOS_LIBOPENCM3_GLUE_OBJECTS}
)
add_dependencies(
  blink_c
  libopencm3
)
target_link_libraries(
  blink_c
  ${LIBOPENCM3_BLUEPILL_LIBRARIES}
)
add_bin_from_elf(
  blink_c.bin
  blink_c
)
add_stlink_upload_target(blink_c.bin)

## Blink in C++ language
add_executable(
  blink_cpp
  blink.cpp
  ${FREERTOS_OBJECTS}
  ${FREERTOS_LIBOPENCM3_GLUE_OBJECTS}
)
add_dependencies(
  blink_cpp
  libopencm3
)
target_link_libraries(
  blink_cpp
  ${LIBOPENCM3_BLUEPILL_LIBRARIES}
)
add_bin_from_elf(
  blink_cpp.bin
  blink_cpp
)
add_stlink_upload_target(blink_cpp.bin)
