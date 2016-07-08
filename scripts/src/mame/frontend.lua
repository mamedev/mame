-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   frontend.lua
--
--   Rules for building frontend
--
---------------------------------------------------------------------------

project ("frontend")
uuid ("e98e14c4-82a4-4988-ba29-01c90c817ab8")
kind (LIBTYPE)

addprojectflags()
precompiledheaders()

if (_OPTIONS["targetos"] ~= "asmjs") then
	options {
		"ArchiveSplit",
	}
end

includedirs {
	MAME_DIR .. "src/osd",
	MAME_DIR .. "src/emu",
	MAME_DIR .. "src/frontend/mame",
	MAME_DIR .. "src/devices", -- till deps are fixed
	MAME_DIR .. "src/lib",
	MAME_DIR .. "src/lib/util",
	MAME_DIR .. "3rdparty/rapidjson/include",
	MAME_DIR .. "3rdparty",
	GEN_DIR  .. "emu",
	GEN_DIR  .. "emu/layout",
}

includedirs {
	ext_includedir("expat"),
	ext_includedir("lua"),
	ext_includedir("zlib"),
	ext_includedir("flac"),
}

if (_OPTIONS["targetos"] == "windows") then
	defines {
		"UI_WINDOWS",
	}
end

if (_OPTIONS["osd"] == "sdl") then
	defines {
		"UI_SDL",
	}
end

files {
	MAME_DIR .. "src/frontend/mame/audit.cpp",
	MAME_DIR .. "src/frontend/mame/audit.h",
	MAME_DIR .. "src/frontend/mame/cheat.cpp",
	MAME_DIR .. "src/frontend/mame/cheat.h",
	MAME_DIR .. "src/frontend/mame/clifront.cpp",
	MAME_DIR .. "src/frontend/mame/clifront.h",
	MAME_DIR .. "src/frontend/mame/info.cpp",
	MAME_DIR .. "src/frontend/mame/info.h",
	MAME_DIR .. "src/frontend/mame/language.cpp",
	MAME_DIR .. "src/frontend/mame/language.h",
	MAME_DIR .. "src/frontend/mame/luaengine.cpp",
	MAME_DIR .. "src/frontend/mame/luaengine.h",
	MAME_DIR .. "src/frontend/mame/mame.cpp",
	MAME_DIR .. "src/frontend/mame/mame.h",
	MAME_DIR .. "src/frontend/mame/mameopts.cpp",
	MAME_DIR .. "src/frontend/mame/mameopts.h",
	MAME_DIR .. "src/frontend/mame/pluginopts.cpp",
	MAME_DIR .. "src/frontend/mame/pluginopts.h",
	MAME_DIR .. "src/frontend/mame/ui/ui.cpp",
	MAME_DIR .. "src/frontend/mame/ui/ui.h",
	MAME_DIR .. "src/frontend/mame/ui/text.cpp",
	MAME_DIR .. "src/frontend/mame/ui/text.h",
	MAME_DIR .. "src/frontend/mame/ui/devctrl.h",
	MAME_DIR .. "src/frontend/mame/ui/menu.cpp",
	MAME_DIR .. "src/frontend/mame/ui/menu.h",
	MAME_DIR .. "src/frontend/mame/ui/submenu.cpp",
	MAME_DIR .. "src/frontend/mame/ui/submenu.h",
	MAME_DIR .. "src/frontend/mame/ui/mainmenu.cpp",
	MAME_DIR .. "src/frontend/mame/ui/mainmenu.h",
	MAME_DIR .. "src/frontend/mame/ui/miscmenu.cpp",
	MAME_DIR .. "src/frontend/mame/ui/miscmenu.h",
	MAME_DIR .. "src/frontend/mame/ui/barcode.cpp",
	MAME_DIR .. "src/frontend/mame/ui/barcode.h",
	MAME_DIR .. "src/frontend/mame/ui/cheatopt.cpp",
	MAME_DIR .. "src/frontend/mame/ui/cheatopt.h",
	MAME_DIR .. "src/frontend/mame/ui/pluginopt.cpp",
	MAME_DIR .. "src/frontend/mame/ui/pluginopt.h",
	MAME_DIR .. "src/frontend/mame/ui/devopt.cpp",
	MAME_DIR .. "src/frontend/mame/ui/devopt.h",
	MAME_DIR .. "src/frontend/mame/ui/filemngr.cpp",
	MAME_DIR .. "src/frontend/mame/ui/filemngr.h",
	MAME_DIR .. "src/frontend/mame/ui/filesel.cpp",
	MAME_DIR .. "src/frontend/mame/ui/filesel.h",
	MAME_DIR .. "src/frontend/mame/ui/filecreate.cpp",
	MAME_DIR .. "src/frontend/mame/ui/filecreate.h",
	MAME_DIR .. "src/frontend/mame/ui/floppycntrl.cpp",
	MAME_DIR .. "src/frontend/mame/ui/floppycntrl.h",
	MAME_DIR .. "src/frontend/mame/ui/imgcntrl.cpp",
	MAME_DIR .. "src/frontend/mame/ui/imgcntrl.h",
	MAME_DIR .. "src/frontend/mame/ui/info.cpp",
	MAME_DIR .. "src/frontend/mame/ui/info.h",
	MAME_DIR .. "src/frontend/mame/ui/info_pty.cpp",
	MAME_DIR .. "src/frontend/mame/ui/info_pty.h",
	MAME_DIR .. "src/frontend/mame/ui/inputmap.cpp",
	MAME_DIR .. "src/frontend/mame/ui/inputmap.h",
	MAME_DIR .. "src/frontend/mame/ui/sliders.cpp",
	MAME_DIR .. "src/frontend/mame/ui/sliders.h",
	MAME_DIR .. "src/frontend/mame/ui/slotopt.cpp",
	MAME_DIR .. "src/frontend/mame/ui/slotopt.h",
	MAME_DIR .. "src/frontend/mame/ui/swlist.cpp",
	MAME_DIR .. "src/frontend/mame/ui/swlist.h",
	MAME_DIR .. "src/frontend/mame/ui/tapectrl.cpp",
	MAME_DIR .. "src/frontend/mame/ui/tapectrl.h",
	MAME_DIR .. "src/frontend/mame/ui/videoopt.cpp",
	MAME_DIR .. "src/frontend/mame/ui/videoopt.h",
	MAME_DIR .. "src/frontend/mame/ui/viewgfx.cpp",
	MAME_DIR .. "src/frontend/mame/ui/viewgfx.h",
	MAME_DIR .. "src/frontend/mame/ui/auditmenu.cpp",
	MAME_DIR .. "src/frontend/mame/ui/auditmenu.h",
	MAME_DIR .. "src/frontend/mame/ui/custmenu.cpp",
	MAME_DIR .. "src/frontend/mame/ui/custmenu.h",
	MAME_DIR .. "src/frontend/mame/ui/custui.cpp",
	MAME_DIR .. "src/frontend/mame/ui/custui.h",
	MAME_DIR .. "src/frontend/mame/ui/datfile.cpp",
	MAME_DIR .. "src/frontend/mame/ui/datfile.h",
	MAME_DIR .. "src/frontend/mame/ui/datmenu.cpp",
	MAME_DIR .. "src/frontend/mame/ui/datmenu.h",
	MAME_DIR .. "src/frontend/mame/ui/defimg.h",
	MAME_DIR .. "src/frontend/mame/ui/dirmenu.cpp",
	MAME_DIR .. "src/frontend/mame/ui/dirmenu.h",
	MAME_DIR .. "src/frontend/mame/ui/icorender.h",
	MAME_DIR .. "src/frontend/mame/ui/inifile.cpp",
	MAME_DIR .. "src/frontend/mame/ui/inifile.h",
	MAME_DIR .. "src/frontend/mame/ui/miscmenu.cpp",
	MAME_DIR .. "src/frontend/mame/ui/miscmenu.h",
	MAME_DIR .. "src/frontend/mame/ui/moptions.cpp",
	MAME_DIR .. "src/frontend/mame/ui/moptions.h",
	MAME_DIR .. "src/frontend/mame/ui/optsmenu.cpp",
	MAME_DIR .. "src/frontend/mame/ui/optsmenu.h",
	MAME_DIR .. "src/frontend/mame/ui/selector.cpp",
	MAME_DIR .. "src/frontend/mame/ui/selector.h",
	MAME_DIR .. "src/frontend/mame/ui/selgame.cpp",
	MAME_DIR .. "src/frontend/mame/ui/selgame.h",
	MAME_DIR .. "src/frontend/mame/ui/simpleselgame.cpp",
	MAME_DIR .. "src/frontend/mame/ui/simpleselgame.h",
	MAME_DIR .. "src/frontend/mame/ui/selsoft.cpp",
	MAME_DIR .. "src/frontend/mame/ui/selsoft.h",
	MAME_DIR .. "src/frontend/mame/ui/sndmenu.cpp",
	MAME_DIR .. "src/frontend/mame/ui/sndmenu.h",
	MAME_DIR .. "src/frontend/mame/ui/starimg.h",
	MAME_DIR .. "src/frontend/mame/ui/toolbar.h",
	MAME_DIR .. "src/frontend/mame/ui/utils.cpp",
	MAME_DIR .. "src/frontend/mame/ui/utils.h",
}
