set(LIBOPENCM3_DIR ${CMAKE_SOURCE_DIR}/thirdparty/libopencm3)

# Make sure that git submodule is initialized and updated
if (NOT EXISTS "${LIBOPENCM3_DIR}/Makefile")
  message(FATAL_ERROR "libopencm3 submodule not found. Initialize with 'git submodule update --init' in the source directory")
endif()

# Add a custom target to compile libopencm3
add_custom_target(libopencm3 make -j4 WORKING_DIRECTORY ${LIBOPENCM3_DIR})
include_directories(${LIBOPENCM3_DIR}/include)
link_directories(${LIBOPENCM3_DIR}/lib)
