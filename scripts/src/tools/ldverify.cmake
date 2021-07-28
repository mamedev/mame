# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
## ldverify
##########################################################################

add_executable(ldverify)

target_include_directories(ldverify PRIVATE
	${MAME_DIR}/src/osd
	${MAME_DIR}/src/lib/util
	${EXT_INCLUDEDIR_FLAC}
)

target_link_libraries(ldverify PRIVATE utils)

target_sources(ldverify PRIVATE
	${MAME_DIR}/src/tools/ldverify.cpp
)

strip_executable(ldverify)
minimal_symbols(ldverify)
