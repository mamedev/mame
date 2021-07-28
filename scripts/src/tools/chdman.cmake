# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
## chdman
##########################################################################

add_executable(chdman)

target_include_directories(chdman PRIVATE
	${MAME_DIR}/src/osd
	${MAME_DIR}/src/lib/util
	${EXT_INCLUDEDIR_FLAC}
)

target_link_libraries(chdman PRIVATE utils)

target_sources(chdman PRIVATE
	${MAME_DIR}/src/tools/chdman.cpp
	${GEN_DIR}/version.cpp
)

set_source_files_properties(${GEN_DIR}/version.cpp PROPERTIES GENERATED TRUE)

strip_executable(chdman)
minimal_symbols(chdman)
