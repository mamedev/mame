# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##################################################
## wdlfft library objects (from Cockos WDL)
##################################################

add_library(wdlfft STATIC EXCLUDE_FROM_ALL)

if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
	target_compile_options(wdlfft PRIVATE -Wno-strict-prototypes)
endif()

target_sources(wdlfft PRIVATE
	${MAME_DIR}/3rdparty/wdlfft/fft.c
	${MAME_DIR}/3rdparty/wdlfft/fft.h
)
