# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
## nltool
##########################################################################

add_executable(nltool)

target_include_directories(nltool PRIVATE
	${MAME_DIR}/src/lib
	${MAME_DIR}/src/lib/netlist
)

if (MINGW)
	target_link_options(nltool PRIVATE -municode)
endif()

target_link_libraries(nltool PRIVATE netlist)

target_sources(nltool PRIVATE
	${MAME_DIR}/src/lib/netlist/prg/nltool.cpp
)

strip_executable(nltool)
minimal_symbols(nltool)
