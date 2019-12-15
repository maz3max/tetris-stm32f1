cmake_minimum_required(VERSION 3.0)
set(CMAKE_TOOLCHAIN_FILE cmake/toolchain-arm-none-eabi.cmake)
project(libopencm3_freertos_bluepill_blink)
include(cmake/libopencm3.cmake)
include(cmake/freertos.cmake)
include(cmake/tetris-board.cmake)
include(cmake/st-link.cmake)

enable_language(ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Targets to build

include_directories("${CMAKE_SOURCE_DIR}/include")

## Tetris (in C language)
add_executable(
  tetris.elf
  src/main.cpp
  src/display.cpp
  src/buttons.cpp
  src/prng.cpp
)
add_dependencies(
  tetris.elf
  libopencm3
)
target_link_libraries(
  tetris.elf
  ${LIBOPENCM3_TETRISBOARD_LIBRARIES}
  freertos
)
add_bin_from_elf(
  tetris.bin
  tetris.elf
)
add_stlink_upload_target(tetris.bin)
