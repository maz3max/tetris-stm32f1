# Make sure that git submodule is initialized and updated
if (NOT EXISTS "${CMAKE_SOURCE_DIR}/freertos/")
  message(FATAL_ERROR "FreeRTOS submodule not found. Initialize with 'git submodule update --init' in the source directory")
endif()

set(FREERTOS_DIR ${CMAKE_SOURCE_DIR}/freertos)
# FIXME Allow to customize with variables
set(FREERTOS_PORTABLE "GCC/ARM_CM3")
set(FREERTOS_MEMORY_MANAGMENT 4)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR} # Make sure FreeRTOSConfig.h is in the source directory
  ${FREERTOS_DIR}/include
  ${FREERTOS_DIR}/portable/${FREERTOS_PORTABLE}
)

# Build objects
add_library(
  tasks
  OBJECT
  ${FREERTOS_DIR}/tasks.c
)
add_library(
  port
  OBJECT
  ${FREERTOS_DIR}/portable/${FREERTOS_PORTABLE}/port.c
)
add_library(
  list
  OBJECT
  ${FREERTOS_DIR}/list.c
)
add_library(
  heap
  OBJECT
  ${FREERTOS_DIR}/portable/MemMang/heap_${FREERTOS_MEMORY_MANAGMENT}.c
)

# Used in top CMakeLists.txt
set(
  FREERTOS_OBJECTS
  $<TARGET_OBJECTS:tasks>
  $<TARGET_OBJECTS:port>
  $<TARGET_OBJECTS:list>
  $<TARGET_OBJECTS:heap>
)
