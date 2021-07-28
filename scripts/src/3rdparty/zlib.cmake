# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##################################################
## zlib library objects
##################################################

add_library(zlib STATIC EXCLUDE_FROM_ALL)

if((CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
	target_compile_options(zlib PRIVATE -Wno-shift-negative-value)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(zlib PRIVATE /wd4131) # warning C4131: 'xxx' : uses old-style declarator
	target_compile_options(zlib PRIVATE /wd4127) # warning C4127: conditional expression is constant
	target_compile_options(zlib PRIVATE /wd4244) # warning C4244: 'argument' : conversion from 'xxx' to 'xxx', possible loss of data
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(zlib PRIVATE verbose=-1)
endif()

if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
	target_compile_options(zlib PRIVATE -Wno-strict-prototypes)
endif()

target_compile_definitions(zlib PRIVATE ZLIB_CONST)

target_sources(zlib PRIVATE
	${MAME_DIR}/3rdparty/zlib/adler32.c
	${MAME_DIR}/3rdparty/zlib/compress.c
	${MAME_DIR}/3rdparty/zlib/crc32.c
	${MAME_DIR}/3rdparty/zlib/deflate.c
	${MAME_DIR}/3rdparty/zlib/inffast.c
	${MAME_DIR}/3rdparty/zlib/inflate.c
	${MAME_DIR}/3rdparty/zlib/infback.c
	${MAME_DIR}/3rdparty/zlib/inftrees.c
	${MAME_DIR}/3rdparty/zlib/trees.c
	${MAME_DIR}/3rdparty/zlib/uncompr.c
	${MAME_DIR}/3rdparty/zlib/zutil.c
)
