# STM32F0 Tetris-Board flags
add_definitions(-DSTM32F1 -DSTM32F103CBT6)
set(STM32F1_FLAGS "-Os -g -mcpu=cortex-m3 -mthumb -msoft-float -MD -specs=nosys.specs")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${STM32F1_FLAGS} -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${STM32F1_FLAGS} -Wall -Wextra -Werror")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--print-memory-usage --static -nostartfiles -MD -T '${CMAKE_SOURCE_DIR}/stm32f103c8t6.ld'")

# Used in top CMakeLists.txt
set(LIBOPENCM3_TETRISBOARD_LIBRARIES opencm3_stm32f1)
