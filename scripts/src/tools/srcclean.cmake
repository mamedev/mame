# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
## srcclean
##########################################################################

add_executable(srcclean)

target_include_directories(srcclean PRIVATE
	${MAME_DIR}/src/osd
	${MAME_DIR}/src/lib/util
)

target_link_libraries(srcclean PRIVATE utils)

target_sources(srcclean PRIVATE
	${MAME_DIR}/src/tools/srcclean.cpp
)

strip_executable(srcclean)
minimal_symbols(srcclean)
