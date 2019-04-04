# STM32F1 Blue-Pill flags
add_definitions(-DSTM32F1)
set(STM32F1_FLAGS "-Os -g -mcpu=cortex-m3 -mthumb -msoft-float -MD -specs=nano.specs -specs=nosys.specs")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${STM32F1_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${STM32F1_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --static -nostartfiles -MD -T '${CMAKE_SOURCE_DIR}/libopencm3/lib/stm32/f1/stm32f103x8.ld'")

# Used in top CMakeLists.txt
set(LIBOPENCM3_BLUEPILL_LIBRARIES opencm3_stm32f1)
