# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
## split
##########################################################################

add_executable(split)

target_include_directories(split PRIVATE
	${MAME_DIR}/src/osd
	${MAME_DIR}/src/lib/util
)

target_link_libraries(split PRIVATE utils)

target_sources(split PRIVATE
	${MAME_DIR}/src/tools/split.cpp
)

strip_executable(split)
minimal_symbols(split)
