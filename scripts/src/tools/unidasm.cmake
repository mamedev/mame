# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
## testkeys
##########################################################################

add_executable(unidasm)

target_include_directories(unidasm PRIVATE
	${MAME_DIR}/src/osd
	${MAME_DIR}/src/devices
	${MAME_DIR}/src/emu
	${MAME_DIR}/src/lib/util
	${MAME_DIR}/3rdparty
)

target_link_libraries(unidasm PRIVATE dasm utils)

target_sources(unidasm PRIVATE
	${MAME_DIR}/src/tools/unidasm.cpp
)
strip_executable(unidasm)
minimal_symbols(unidasm)
