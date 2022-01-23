# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
## floptool
##########################################################################

add_executable(floptool)

target_include_directories(floptool PRIVATE
	${MAME_DIR}/src/osd
	${MAME_DIR}/src/lib
	${MAME_DIR}/src/lib/util
)

target_link_libraries(floptool PRIVATE formats)

target_sources(floptool PRIVATE
	${MAME_DIR}/src/tools/image_handler.cpp
	${MAME_DIR}/src/tools/image_handler.h
	${MAME_DIR}/src/tools/floptool.cpp
	${GEN_DIR}/version.cpp
)

strip_executable(floptool)
minimal_symbols(floptool)
