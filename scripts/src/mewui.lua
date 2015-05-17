if (_OPTIONS["targetos"] == "windows") then
	defines {
		"MEWUI_WINDOWS",
	}
end

if (_OPTIONS["osd"] == "sdl") then
	defines {
		"MEWUI_SDL",
	}
end

files {
	MAME_DIR .. "src/emu/mewui/selgame.c",
	MAME_DIR .. "src/emu/mewui/selgame.h",
}
