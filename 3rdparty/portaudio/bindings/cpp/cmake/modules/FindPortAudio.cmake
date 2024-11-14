
macro(handle_default)

endmacro()

if(TARGET PortAudio::portaudio)
  # nothing to do 
  return()
endif()
# search for portaudio as cmake module
find_package(PortAudio CONFIG QUIET)
if(PortAudio_FOUND)
  if(TARGET PortAudio::portaudio)  
    return()  
  elseif(TARGET portaudio)
    # vcpkg and old portaudio installations do not provide the same targets
    add_library(PortAudio::portaudio ALIAS portaudio)
    return()
  else()
    message(FATAL_ERROR "PortAudio_FOUND but not target PortAudio::portaudio")  
  endif()  
endif()

# search for portaudio via pkg-config

message(STATUS "portaudio could not be found via cmake, specify PortAudio_DIR.\n Searching for it via pkg-config")
find_package(PkgConfig REQUIRED)
pkg_check_modules(portaudio REQUIRED QUIET IMPORTED_TARGET GLOBAL portaudio-2.0)
add_library(PortAudio::portaudio ALIAS PkgConfig::portaudio)
return()

# include(FindPackageHandleStandardArgs)
# find_package_handle_standard_args(Foo
#   FOUND_VAR Foo_FOUND
#   REQUIRED_VARS
#     Foo_LIBRARY
#     Foo_INCLUDE_DIR
#   VERSION_VAR Foo_VERSION
# )
