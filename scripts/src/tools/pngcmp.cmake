# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
## pngcmp
##########################################################################

add_executable(pngcmp)

target_include_directories(pngcmp PRIVATE
	${MAME_DIR}/src/osd
	${MAME_DIR}/src/lib/util
)

target_link_libraries(pngcmp PRIVATE utils)

target_sources(pngcmp PRIVATE
	${MAME_DIR}/src/tools/pngcmp.cpp
)

strip_executable(pngcmp)
minimal_symbols(pngcmp)
