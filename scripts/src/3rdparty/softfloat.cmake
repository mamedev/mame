# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##################################################
## SoftFloat library objects
##################################################

set(SOFTFLOAT_SRCS
	${MAME_DIR}/3rdparty/softfloat/softfloat.c
	${MAME_DIR}/3rdparty/softfloat/fsincos.c
	${MAME_DIR}/3rdparty/softfloat/fpatan.c
	${MAME_DIR}/3rdparty/softfloat/fyl2x.c
)

set_source_files_properties(${SOFTFLOAT_SRCS} PROPERTIES LANGUAGE "CXX")

add_library(softfloat STATIC EXCLUDE_FROM_ALL ${SOFTFLOAT_SRCS})

target_include_directories(softfloat PRIVATE ${MAME_DIR}/src/osd)

if(((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang")) AND NOT MSVC)
	target_compile_options(softfloat PRIVATE -x c++)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(softfloat PRIVATE /wd4244) # warning C4244: 'argument' : conversion from 'xxx' to 'xxx', possible loss of data
	target_compile_options(softfloat PRIVATE /wd4146) # warning C4146: unary minus operator applied to unsigned type, result still unsigned
	target_compile_options(softfloat PRIVATE /wd4018) # warning C4018: 'x' : signed/unsigned mismatch
endif()
