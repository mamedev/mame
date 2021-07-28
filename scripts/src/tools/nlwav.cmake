# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
## nlwav
##########################################################################

add_executable(nlwav)

target_include_directories(nlwav PRIVATE
	${MAME_DIR}/src/lib
	${MAME_DIR}/src/lib/netlist
)

if (MINGW)
	target_link_options(nlwav PRIVATE -municode)
endif()

target_link_libraries(nlwav PRIVATE netlist)

target_sources(nlwav PRIVATE
	${MAME_DIR}/src/lib/netlist/prg/nlwav.cpp
)

strip_executable(nlwav)
minimal_symbols(nlwav)
