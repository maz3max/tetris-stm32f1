cmake_minimum_required(VERSION 3.0)
set(CMAKE_TOOLCHAIN_FILE cmake/toolchain-arm-none-eabi.cmake)
project(libopencm3_freertos_bluepill_blink)
include(cmake/blue-pill.cmake)
include(cmake/libopencm3.cmake)
include(cmake/freertos.cmake)
include(cmake/freertos-libopencm3-glue.cmake)
include(cmake/st-link.cmake)

# Targets to build
add_executable(
  blink
  blink.c
  ${FREERTOS_OBJECTS}
  ${FREERTOS_LIBOPENCM3_GLUE_OBJECTS}
)
add_dependencies(
  blink
  ${LIBOPENCM3_BLUEPILL_DEPENDENCIES}
)
target_link_libraries(
  blink
  ${LIBOPENCM3_BLUEPILL_LIBRARIES}
)
add_bin_from_elf(
  blink.bin
  blink
)
add_stlink_upload_target(blink.bin)
