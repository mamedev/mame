# static.cmake -- change flags to link with static runtime libraries
#
# Even when BUILD_SHARED_LIBS is OFF, CMake specifies linking wtih
# multithread DLL, so you give inconsistent linking instructions
# resulting in warning messages from MS Visual Studio. If you want
# a static binary, I've found this approach works to eliminate
# warnings and make everything static:
#
# Changes /MD (multithread DLL) to /MT (multithread static)

if(MSVC)
  foreach(flag_var
    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
    CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
    CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif(${flag_var} MATCHES "/MD")
  endforeach(flag_var)
 
  message(STATUS 
    "Note: overriding CMAKE_*_FLAGS_* to use Visual C static multithread library")
endif(MSVC)
