set(CMAKE_SYSTEM_NAME Generic)

# Find the compiler and corresponding tools
find_program(ARM_CC arm-none-eabi-gcc)
if (NOT ARM_CC)
  message(FATAL_ERROR "arm-none-eabi-gcc not found")
endif()

find_program(ARM_CXX arm-none-eabi-g++)
if (NOT ARM_CXX)
  message(FATAL_ERROR "arm-none-eabi-g++ not found")
endif()

find_program(ARM_OBJCOPY arm-none-eabi-objcopy)
if (NOT ARM_OBJCOPY)
  message(FATAL_ERROR "arm-none-eabi-objcopy not found")
endif()

find_program(ARM_SIZE_TOOL arm-none-eabi-size)
if (NOT ARM_SIZE_TOOL)
  message(FATAL_ERROR "arm-none-eabi-siz not found")
endif()

find_program(ARM_AS arm-none-eabi-as)
if (NOT ARM_AS)
  message(FATAL_ERROR "arm-none-eabi-as not found")
endif()

find_program(ARM_AR arm-none-eabi-ar)
if (NOT ARM_AR)
  message(FATAL_ERROR "arm-none-eabi-ar not found")
endif()

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

# Don't look for system programs, includes, libraries
# and packages because we are cross-compiling
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER)

# Function to generate bin files from elf files
function(add_bin_from_elf bin elf)
  add_custom_target(${bin} ALL ${CMAKE_OBJCOPY} -Obinary ${elf} ${bin} DEPENDS ${elf})
endfunction(add_bin_from_elf)
