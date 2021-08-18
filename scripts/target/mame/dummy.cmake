# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
##
##   dummy.lua
##
##   Dummy target makefile
##
##########################################################################

include(scripts/target/mame/arcade.cmake)
include(scripts/target/mame/mess.cmake)

macro(createProjects_mame_dummy _target  _subtarget)
	add_library(mame_dummy ${LIBTYPE})
	addprojectflags(mame_dummy)
	precompiledheaders_novs(mame_dummy)
	add_dependencies(mame_dummy layouts optional)

	target_include_directories(mame_dummy PRIVATE
		${MAME_DIR}/src/osd
		${MAME_DIR}/src/emu
		${MAME_DIR}/src/devices
		${MAME_DIR}/src/mame
		${MAME_DIR}/src/lib
		${MAME_DIR}/src/lib/util
		${MAME_DIR}/3rdparty
		${GEN_DIR}/mame/layout
	)

	target_sources(mame_dummy PRIVATE
		${MAME_DIR}/src/mame/drivers/coleco.cpp
		${MAME_DIR}/src/mame/includes/coleco.h
		${MAME_DIR}/src/mame/machine/coleco.cpp
		${MAME_DIR}/src/mame/machine/coleco.h
	)
	add_project_to_group(drivers mame_dummy)
endmacro()

macro(linkProjects_mame_dummy _target _subtarget _projectname)
	target_link_libraries(${_projectname} PRIVATE mame_dummy)
endmacro()
