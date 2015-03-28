dofile("../mess/tiny.lua")
dofile("../mame/tiny.lua")

function createProjects_ume_tiny(_target, _subtarget)
	createProjects_mess_tiny(_target, _subtarget)
	createProjects_mame_tiny(_target, _subtarget)
end

function linkProjects_ume_tiny(_target, _subtarget)
	linkProjects_mess_tiny(_target, _subtarget)
	linkProjects_mame_tiny(_target, _subtarget)
end