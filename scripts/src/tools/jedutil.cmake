# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
## jedutil
##########################################################################

add_executable(jedutil)

target_include_directories(jedutil PRIVATE
	${MAME_DIR}/src/osd
	${MAME_DIR}/src/lib/util
)

target_link_libraries(jedutil PRIVATE utils)

target_sources(jedutil PRIVATE
	${MAME_DIR}/src/tools/jedutil.cpp
)

strip_executable(jedutil)
minimal_symbols(jedutil)
