# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
##
##   mame.cmake
##
##   MAME target makefile
##
##########################################################################

include(scripts/target/mame/arcade.cmake)
include(scripts/target/mame/mess.cmake)
include(scripts/target/mame/virtual.cmake)

macro(createProjects_mame_mame _target _subtarget)
	createProjects_mame_arcade(${_target} ${_subtarget})
	createProjects_mame_mess(${_target} ${_subtarget})
	createProjects_mame_virtual(${_target} ${_subtarget})
endmacro()

macro(linkProjects_mame_mame _target _subtarget _projectname)
	linkProjects_mame_arcade(${_target} ${_subtarget} ${_projectname})
	linkProjects_mame_mess(${_target} ${_subtarget} ${_projectname})
	linkProjects_mame_virtual(${_target} ${_subtarget} ${_projectname})
endmacro()
