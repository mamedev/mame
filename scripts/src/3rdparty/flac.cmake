# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##################################################
## libflac library objects
##################################################

add_library(flac STATIC EXCLUDE_FROM_ALL)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	target_compile_options(flac PRIVATE /wd4127) # warning C4127: conditional expression is constant
	target_compile_options(flac PRIVATE /wd4244) # warning C4244: 'argument' : conversion from 'xxx' to 'xxx', possible loss of data
	target_compile_options(flac PRIVATE /wd4100) # warning C4100: 'xxx' : unreferenced formal parameter
	target_compile_options(flac PRIVATE /wd4456) # warning C4456: declaration of 'xxx' hides previous local declaration
	target_compile_options(flac PRIVATE /wd4702) # warning C4702: unreachable code
endif()

target_compile_definitions(flac PRIVATE
	WORDS_BIGENDIAN=0
	FLAC__NO_ASM
	_LARGEFILE_SOURCE
	_FILE_OFFSET_BITS=64
	FLAC__HAS_OGG=0
	HAVE_CONFIG_H=1
)

if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
	target_compile_options(flac PRIVATE -Wno-unused-function)
	if(NOT MSVC)
		target_compile_options(flac PRIVATE -O0)
	endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	target_compile_options(flac PRIVATE -Wno-enum-conversion)
	if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
		target_compile_options(flac PRIVATE -Wno-unknown-attributes)
	endif()
endif()

target_include_directories(flac PRIVATE
	${MAME_DIR}/3rdparty/libflac/src/libFLAC/include
	${MAME_DIR}/3rdparty/libflac/include
)

target_sources(flac PRIVATE
	${MAME_DIR}/3rdparty/libflac/src/libFLAC/bitmath.c
	${MAME_DIR}/3rdparty/libflac/src/libFLAC/bitreader.c
	${MAME_DIR}/3rdparty/libflac/src/libFLAC/bitwriter.c
	${MAME_DIR}/3rdparty/libflac/src/libFLAC/cpu.c
	${MAME_DIR}/3rdparty/libflac/src/libFLAC/crc.c
	${MAME_DIR}/3rdparty/libflac/src/libFLAC/fixed.c
	${MAME_DIR}/3rdparty/libflac/src/libFLAC/float.c
	${MAME_DIR}/3rdparty/libflac/src/libFLAC/format.c
	${MAME_DIR}/3rdparty/libflac/src/libFLAC/lpc.c
	${MAME_DIR}/3rdparty/libflac/src/libFLAC/md5.c
	${MAME_DIR}/3rdparty/libflac/src/libFLAC/memory.c
	${MAME_DIR}/3rdparty/libflac/src/libFLAC/stream_decoder.c
	${MAME_DIR}/3rdparty/libflac/src/libFLAC/stream_encoder.c
	${MAME_DIR}/3rdparty/libflac/src/libFLAC/stream_encoder_framing.c
	${MAME_DIR}/3rdparty/libflac/src/libFLAC/window.c
)

target_compile_definitions(flac PUBLIC FLAC__NO_DLL)
