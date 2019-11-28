# STM32F0 Tetris-Board flags
add_definitions(-DSTM32F0)
set(STM32F0_FLAGS "-Os -g -mcpu=cortex-m0 -mthumb -msoft-float -MD -specs=nano.specs -specs=nosys.specs")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${STM32F0_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${STM32F0_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --static -nostartfiles -MD -T '${CMAKE_SOURCE_DIR}/libopencm3/lib/stm32/f0/stm32f03xz6.ld'")

# Used in top CMakeLists.txt
set(LIBOPENCM3_TETRISBOARD_LIBRARIES opencm3_stm32f0)
