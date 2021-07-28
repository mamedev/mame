# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##################################################
## SQLite3 library objects
##################################################

add_library(sqlite3 STATIC EXCLUDE_FROM_ALL)

if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
	target_compile_options(sqlite3 PRIVATE -Wno-bad-function-cast)
	target_compile_options(sqlite3 PRIVATE -Wno-discarded-qualifiers)
	target_compile_options(sqlite3 PRIVATE -Wno-undef)
	target_compile_options(sqlite3 PRIVATE -Wno-unused-but-set-variable)
	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		target_compile_options(sqlite3 PRIVATE -Wno-incompatible-pointer-types-discards-qualifiers)
	endif()
endif()

target_sources(sqlite3 PRIVATE
	${MAME_DIR}/3rdparty/sqlite3/sqlite3.c
)
