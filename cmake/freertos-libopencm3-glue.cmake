# libopencm3 and FreeRTOS includes must be
# known before running this CMake script
add_library(
  freertos_libopencm3_glue
  OBJECT
  freertos_libopencm3/glue.c
)

# Used in top CMakeLists.txt
set(
  FREERTOS_LIBOPENCM3_GLUE_OBJECTS
  $<TARGET_OBJECTS:freertos_libopencm3_glue>
)
