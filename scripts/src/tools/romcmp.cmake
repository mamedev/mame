# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
## romcmp
##########################################################################

add_executable(romcmp)

target_include_directories(romcmp PRIVATE
	${MAME_DIR}/src/osd
	${MAME_DIR}/src/lib/util
)

target_link_libraries(romcmp PRIVATE utils)

target_sources(romcmp PRIVATE
	${MAME_DIR}/src/tools/romcmp.cpp
)

strip_executable(romcmp)
minimal_symbols(romcmp)
