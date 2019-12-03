set(FREERTOS_DIR ${CMAKE_SOURCE_DIR}/thirdparty/freertos)

# Make sure that git submodule is initialized and updated
if (NOT EXISTS "${FREERTOS_DIR}")
  message(FATAL_ERROR "FreeRTOS submodule not found. Initialize with 'git submodule update --init' in the source directory")
endif()

set(FREERTOS_PORTABLE "GCC/ARM_CM0" CACHE STRING "Portable definitions")
set(FREERTOS_MEMORY_MANAGMENT 4 CACHE STRING "https://www.freertos.org/a00111.html")
set_property(CACHE FREERTOS_MEMORY_MANAGMENT PROPERTY STRINGS 1 2 3 4 5)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include # Make sure FreeRTOSConfig.h is in the source directory
  ${FREERTOS_DIR}/include
  ${FREERTOS_DIR}/portable/${FREERTOS_PORTABLE}
)

# Build objects
add_library(
  freertos
  STATIC
  ${FREERTOS_DIR}/portable/MemMang/heap_${FREERTOS_MEMORY_MANAGMENT}.c
  ${FREERTOS_DIR}/portable/${FREERTOS_PORTABLE}/port.c
  ${FREERTOS_DIR}/tasks.c
  ${FREERTOS_DIR}/list.c
  ${FREERTOS_DIR}/queue.c
  ${FREERTOS_DIR}/timers.c
  ${FREERTOS_DIR}/stream_buffer.c
  ${FREERTOS_DIR}/event_groups.c
  ${FREERTOS_DIR}/croutine.c
)
