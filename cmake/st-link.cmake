# Function to upload the target using ST-Link and st-flash from texane/stlink
find_program(ST-FLASH st-flash)

# Allows to add a target to upload the program via ST-Link
# The target must be called explicitly
function(add_stlink_upload_target bin)
  if (NOT ST-FLASH)
    message(FATAL_ERROR "st-flash not found.")
  endif()
  add_custom_target(${bin}_upload ALL st-flash write ${bin} 0x8000000 DEPENDS ${bin})
  set_target_properties(${bin}_upload PROPERTIES EXCLUDE_FROM_ALL TRUE)
endfunction(add_stlink_upload_target)
