# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
## ledutil
##########################################################################

add_executable(ledutil)

target_sources(ledutil PRIVATE
	${MAME_DIR}/src/osd/windows/ledutil.cpp
)

target_include_directories(ledutil PRIVATE
	${MAME_DIR}/src/osd
)

target_link_libraries(ledutil PRIVATE ocore_windows)

strip_executable(ledutil)
minimal_symbols(ledutil)
