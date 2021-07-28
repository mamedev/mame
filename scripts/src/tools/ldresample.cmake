# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
## ldresample
##########################################################################

add_executable(ldresample)

target_include_directories(ldresample PRIVATE
	${MAME_DIR}/src/osd
	${MAME_DIR}/src/lib/util
	${EXT_INCLUDEDIR_FLAC}
)

target_link_libraries(ldresample PRIVATE utils)

target_sources(ldresample PRIVATE
	${MAME_DIR}/src/tools/ldresample.cpp
)

strip_executable(ldresample)
minimal_symbols(ldresample)
