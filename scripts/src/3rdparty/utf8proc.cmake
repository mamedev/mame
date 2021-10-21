# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##################################################
## utf8proc library objects
##################################################

add_library(utf8proc STATIC EXCLUDE_FROM_ALL)

target_compile_definitions(utf8proc PRIVATE UTF8PROC_STATIC)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(utf8proc PRIVATE verbose=-1)
endif()

if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
	target_compile_options(utf8proc PRIVATE -Wno-strict-prototypes)
endif()

target_sources(utf8proc PRIVATE
	${MAME_DIR}/3rdparty/utf8proc/utf8proc.c
)

