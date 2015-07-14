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
	MAME_DIR .. "src/emu/mewui/auditmenu.c",
	MAME_DIR .. "src/emu/mewui/auditmenu.h",
	MAME_DIR .. "src/emu/mewui/ctrlmenu.c",
	MAME_DIR .. "src/emu/mewui/ctrlmenu.h",
	MAME_DIR .. "src/emu/mewui/custmenu.c",
	MAME_DIR .. "src/emu/mewui/custmenu.h",
	MAME_DIR .. "src/emu/mewui/custui.c",
	MAME_DIR .. "src/emu/mewui/custui.h",
	MAME_DIR .. "src/emu/mewui/datfile.c",
	MAME_DIR .. "src/emu/mewui/datfile.h",
	MAME_DIR .. "src/emu/mewui/datmenu.c",
	MAME_DIR .. "src/emu/mewui/datmenu.h",
	MAME_DIR .. "src/emu/mewui/dirmenu.c",
	MAME_DIR .. "src/emu/mewui/dirmenu.h",
	MAME_DIR .. "src/emu/mewui/dsplmenu.c",
	MAME_DIR .. "src/emu/mewui/dsplmenu.h",
	MAME_DIR .. "src/emu/mewui/inifile.c",
	MAME_DIR .. "src/emu/mewui/inifile.h",
	MAME_DIR .. "src/emu/mewui/miscmenu.c",
	MAME_DIR .. "src/emu/mewui/miscmenu.h",
	MAME_DIR .. "src/emu/mewui/moptions.c",
	MAME_DIR .. "src/emu/mewui/moptions.h",
	MAME_DIR .. "src/emu/mewui/optsmenu.c",
	MAME_DIR .. "src/emu/mewui/optsmenu.h",
	MAME_DIR .. "src/emu/mewui/palsel.c",
	MAME_DIR .. "src/emu/mewui/palsel.h",
	MAME_DIR .. "src/emu/mewui/selector.c",
	MAME_DIR .. "src/emu/mewui/selector.h",
	MAME_DIR .. "src/emu/mewui/selgame.c",
	MAME_DIR .. "src/emu/mewui/selgame.h",
	MAME_DIR .. "src/emu/mewui/selsoft.c",
	MAME_DIR .. "src/emu/mewui/selsoft.h",
	MAME_DIR .. "src/emu/mewui/sndmenu.c",
	MAME_DIR .. "src/emu/mewui/sndmenu.h",
	MAME_DIR .. "src/emu/mewui/swcustmenu.c",
	MAME_DIR .. "src/emu/mewui/swcustmenu.h",
	MAME_DIR .. "src/emu/mewui/toolbar.h",
	MAME_DIR .. "src/emu/mewui/utils.c",
	MAME_DIR .. "src/emu/mewui/utils.h",
}

dependency {
	-- MEWUI
	{ MAME_DIR .. "src/emu/rendfont.c", GEN_DIR .. "emu/mewui/uicmd14.fh" },
}

custombuildtask {
	-- MEWUI
	{ MAME_DIR .. "src/emu/mewui/uicmd14.png"	, GEN_DIR .. "emu/mewui/uicmd14.fh",  {  MAME_DIR.. "src/build/png2bdc.py",  MAME_DIR .. "src/build/file2str.py" }, {"@echo Converting uicmd14.png...", "python $(1) $(<) temp_cmd.bdc", "python $(2) temp_cmd.bdc $(@) font_uicmd14 UINT8" }},
}
