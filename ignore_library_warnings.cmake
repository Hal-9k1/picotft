function(ignore_library_warnings library_name)
  get_target_property(_library_inc_dirs ${library_name} INTERFACE_INCLUDE_DIRECTORIES)
  target_include_directories(${library_name} SYSTEM INTERFACE ${_library_inc_dirs})
endfunction()
