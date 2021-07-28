# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
## regrep
##########################################################################

add_executable(regrep)

target_include_directories(regrep PRIVATE
	${MAME_DIR}/src/osd
	${MAME_DIR}/src/lib/util
)

target_link_libraries(regrep PRIVATE utils)

target_sources(regrep PRIVATE
	${MAME_DIR}/src/tools/regrep.cpp
)

strip_executable(regrep)
minimal_symbols(regrep)
