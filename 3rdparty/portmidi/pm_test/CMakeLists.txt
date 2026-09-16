# CMake file to build tests in this directory: pm_test

# set the build directory to be in portmidi, not in portmidi/pm_test
# this is required for Xcode:
if(APPLE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
endif(APPLE)

# if(WIN32)
#   if(NOT BUILD_SHARED_LIBS)
    # /MDd is multithread debug DLL, /MTd is multithread debug
    # /MD is multithread DLL, /MT is multithread. Change to static:
#     include(../pm_win/static.cmake)
#   endif()
# endif(WIN32)

if(HAIKU)
    add_compile_options(-fPIC) # Haiku x86_64 needs this explicitly
endif()

macro(add_test name)
  add_executable(${name} ${name}.c)
  target_link_libraries(${name} PRIVATE portmidi)
  set_property(TARGET ${name} PROPERTY MSVC_RUNTIME_LIBRARY
               "MultiThreaded$<$<CONFIG:Debug>:Debug>${MSVCRT_DLL}")
endmacro(add_test)

add_test(testio)
add_test(midithread)
add_test(midithru)
add_test(sysex)
add_test(latency)
add_test(mm)
add_test(midiclock)
add_test(qtest)
add_test(fast)
add_test(fastrcv)
add_test(pmlist)
if(WIN32)
# windows does not implement Pm_CreateVirtualInput or Pm_CreateVirtualOutput
else(WIN32)
add_test(recvvirtual)
add_test(sendvirtual)
add_test(multivirtual)
add_test(virttest)
endif(WIN32)
