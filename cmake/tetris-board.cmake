# STM32F0 Tetris-Board flags
add_definitions(-DSTM32F1 -DSTM32F103CBT6)
set(STM32F1_FLAGS "-Os -g -mcpu=cortex-m3 -mthumb -msoft-float -MD -specs=nano.specs -specs=nosys.specs")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${STM32F1_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${STM32F1_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--print-memory-usage --static -nostartfiles -MD -T '${LIBOPENCM3_DIR}/lib/stm32/f1/stm32f103xb.ld'")

# Used in top CMakeLists.txt
set(LIBOPENCM3_TETRISBOARD_LIBRARIES opencm3_stm32f1)
