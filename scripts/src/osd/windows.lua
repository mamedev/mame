function includeosd()
	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/osd/windows",
	}
end


forcedincludes {
	MAME_DIR .. "src/osd/windows/winprefix.h"
}


project ("osd_" .. _OPTIONS["osd"])
	uuid (os.uuid("osd_" .. _OPTIONS["osd"]))
	kind "StaticLib"

	removeflags {
		"SingleOutputDir",
	}
	
	options {
		"ForceCPP",
	}

	dofile("windows_cfg.lua")
	
	defines {
		"DIRECTINPUT_VERSION=0x0800",
		"DIRECT3D_VERSION=0x0900",
	}

	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "src/osd/modules/render",
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "3rdparty/winpcap/Include",
		MAME_DIR .. "3rdparty/bgfx/include",
		MAME_DIR .. "3rdparty/bx/include",
	}

	includedirs {
		MAME_DIR .. "src/osd/windows",
	}

	files {
		MAME_DIR .. "src/osd/modules/render/drawd3d.c",
		MAME_DIR .. "src/osd/modules/render/d3d/d3d9intf.c",
		MAME_DIR .. "src/osd/modules/render/d3d/d3dhlsl.c",
		MAME_DIR .. "src/osd/modules/render/drawdd.c",
		MAME_DIR .. "src/osd/modules/render/drawgdi.c",
		MAME_DIR .. "src/osd/modules/render/drawbgfx.c",
		MAME_DIR .. "src/osd/modules/render/drawnone.c",
		MAME_DIR .. "src/osd/windows/input.c",
		MAME_DIR .. "src/osd/windows/output.c",
		MAME_DIR .. "src/osd/windows/video.c",
		MAME_DIR .. "src/osd/windows/window.c",
		MAME_DIR .. "src/osd/windows/winmenu.c",
		MAME_DIR .. "src/osd/windows/winmain.c",
		MAME_DIR .. "src/osd/modules/debugger/none.*",
		MAME_DIR .. "src/osd/modules/debugger/debugint.*",
		MAME_DIR .. "src/osd/modules/debugger/debugwin.*",
		MAME_DIR .. "src/osd/modules/debugger/debugqt.*",
		MAME_DIR .. "src/osd/modules/debugger/win/*.*",
		MAME_DIR .. "src/osd/modules/render/drawogl.*",
		MAME_DIR .. "src/osd/modules/opengl/gl_shader_tool.*",
		MAME_DIR .. "src/osd/modules/opengl/gl_shader_mgr.*",
		MAME_DIR .. "src/osd/modules/lib/osdobj_common.*",
		MAME_DIR .. "src/osd/modules/font/font_sdl.*",
		MAME_DIR .. "src/osd/modules/font/font_windows.*",
		MAME_DIR .. "src/osd/modules/font/font_osx.*",
		MAME_DIR .. "src/osd/modules/font/font_none.*",
		MAME_DIR .. "src/osd/modules/netdev/taptun.*",
		MAME_DIR .. "src/osd/modules/netdev/pcap.*",
		MAME_DIR .. "src/osd/modules/netdev/none.*",
		MAME_DIR .. "src/osd/modules/midi/portmidi.*",
		MAME_DIR .. "src/osd/modules/midi/none.*",
		MAME_DIR .. "src/osd/modules/sound/js_sound.*",
		MAME_DIR .. "src/osd/modules/sound/direct_sound.*",
		MAME_DIR .. "src/osd/modules/sound/sdl_sound.*",
		MAME_DIR .. "src/osd/modules/sound/none.*",
	}
	
project ("ocore_" .. _OPTIONS["osd"])
	uuid (os.uuid("ocore_" .. _OPTIONS["osd"]))
	kind "StaticLib"

	options {
		"ForceCPP",
	}
	removeflags {
		"SingleOutputDir",	
	}

	dofile("windows_cfg.lua")
	
	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
	}

	--if _OPTIONS["targetos"]=="linux" then
	--	BASE_TARGETOS = "unix"
	--	SDLOS_TARGETOS = "unix"
	--	SYNC_IMPLEMENTATION = "tc"
	--end

	--if _OPTIONS["targetos"]=="windows" then
		BASE_TARGETOS = "win32"
		SDLOS_TARGETOS = "win32"
		SYNC_IMPLEMENTATION = "windows"
	--end

	--if _OPTIONS["targetos"]=="macosx" then
	--	BASE_TARGETOS = "unix"
	--	SDLOS_TARGETOS = "macosx"
	--	SYNC_IMPLEMENTATION = "ntc"
	--end

	includedirs {
		MAME_DIR .. "src/osd/windows",
		MAME_DIR .. "src/lib/winpcap",
	}

	files {
		MAME_DIR .. "src/osd/modules/osdmodule.*",
		MAME_DIR .. "src/osd/windows/main.c",
		MAME_DIR .. "src/osd/windows/strconv.c",
		MAME_DIR .. "src/osd/windows/windir.c",
		MAME_DIR .. "src/osd/windows/winfile.c",
		MAME_DIR .. "src/osd/modules/sync/sync_windows.c",
		MAME_DIR .. "src/osd/windows/winutf8.c",
		MAME_DIR .. "src/osd/windows/winutil.c",
		MAME_DIR .. "src/osd/windows/winclip.c",
		MAME_DIR .. "src/osd/windows/winsocket.c",
		MAME_DIR .. "src/osd/modules/sync/work_osd.c",
		MAME_DIR .. "src/osd/modules/lib/osdlib_win32.c",
		MAME_DIR .. "src/osd/windows/winptty.c",
	}
