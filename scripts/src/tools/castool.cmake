# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
## castool
##########################################################################

add_executable(castool)

target_include_directories(castool PRIVATE
	${MAME_DIR}/src/osd
	${MAME_DIR}/src/lib
	${MAME_DIR}/src/lib/util
)

target_link_libraries(castool PRIVATE formats)

target_sources(castool PRIVATE
	${MAME_DIR}/src/tools/castool.cpp
)

strip_executable(castool)
minimal_symbols(castool)
