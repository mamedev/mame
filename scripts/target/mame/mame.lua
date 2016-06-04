-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   mame.lua
--
--   MAME target makefile
--
---------------------------------------------------------------------------

dofile("arcade.lua")
dofile("mess.lua")

function createProjects_mame_mame(_target, _subtarget)
	createProjects_mame_arcade(_target, _subtarget)
	createProjects_mame_mess(_target, _subtarget)
end

function linkProjects_mame_mame(_target, _subtarget)
	linkProjects_mame_arcade(_target, _subtarget)
	linkProjects_mame_mess(_target, _subtarget)
end
