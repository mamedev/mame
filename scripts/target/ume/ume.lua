---------------------------------------------------------------------------
--
--   ume.lua
--
--   Universal target makefile
--
---------------------------------------------------------------------------

dofile("../mess/mess.lua")
dofile("../mame/mame.lua")

function createProjects_ume_ume(_target, _subtarget)
	createProjects_mess_mess(_target, _subtarget)
	createProjects_mame_mame(_target, _subtarget)
end

function linkProjects_ume_ume(_target, _subtarget)
	linkProjects_mess_mess(_target, _subtarget)
	linkProjects_mame_mame(_target, _subtarget)
end