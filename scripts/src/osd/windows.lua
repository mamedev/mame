function maintargetosdoptions(_target)
	local rcfile = MAME_DIR .. "src/" .. _target .. "/osd/windows/" .. _target ..".rc"

	if os.isfile(rcfile) then
		files {
			rcfile,
		}
	else
		files {
			MAME_DIR .. "src/osd/windows/mame.rc",
		}
	end
end


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
		MAME_DIR .. "src/osd/modules/debugger/none.c",
		MAME_DIR .. "src/osd/modules/debugger/debugint.c",
		MAME_DIR .. "src/osd/modules/debugger/debugwin.c",
		MAME_DIR .. "src/osd/modules/debugger/debugqt.c",
		MAME_DIR .. "src/osd/modules/debugger/win/consolewininfo.c",
		MAME_DIR .. "src/osd/modules/debugger/win/debugbaseinfo.c",
		MAME_DIR .. "src/osd/modules/debugger/win/debugviewinfo.c",
		MAME_DIR .. "src/osd/modules/debugger/win/debugwininfo.c",
		MAME_DIR .. "src/osd/modules/debugger/win/disasmbasewininfo.c",
		MAME_DIR .. "src/osd/modules/debugger/win/disasmviewinfo.c",
		MAME_DIR .. "src/osd/modules/debugger/win/disasmwininfo.c",
		MAME_DIR .. "src/osd/modules/debugger/win/editwininfo.c",
		MAME_DIR .. "src/osd/modules/debugger/win/logwininfo.c",
		MAME_DIR .. "src/osd/modules/debugger/win/memoryviewinfo.c",
		MAME_DIR .. "src/osd/modules/debugger/win/memorywininfo.c",
		MAME_DIR .. "src/osd/modules/debugger/win/pointswininfo.c",
		MAME_DIR .. "src/osd/modules/debugger/win/uimetrics.c",
		MAME_DIR .. "src/osd/modules/render/drawogl.c",
		MAME_DIR .. "src/osd/modules/opengl/gl_shader_tool.c",
		MAME_DIR .. "src/osd/modules/opengl/gl_shader_mgr.c",
		MAME_DIR .. "src/osd/modules/lib/osdobj_common.c",
		MAME_DIR .. "src/osd/modules/font/font_sdl.c",
		MAME_DIR .. "src/osd/modules/font/font_windows.c",
		MAME_DIR .. "src/osd/modules/font/font_osx.c",
		MAME_DIR .. "src/osd/modules/font/font_none.c",
		MAME_DIR .. "src/osd/modules/netdev/taptun.c",
		MAME_DIR .. "src/osd/modules/netdev/pcap.c",
		MAME_DIR .. "src/osd/modules/netdev/none.c",
		MAME_DIR .. "src/osd/modules/midi/portmidi.c",
		MAME_DIR .. "src/osd/modules/midi/none.c",
		MAME_DIR .. "src/osd/modules/sound/js_sound.c",
		MAME_DIR .. "src/osd/modules/sound/direct_sound.c",
		MAME_DIR .. "src/osd/modules/sound/sdl_sound.c",
		MAME_DIR .. "src/osd/modules/sound/none.c",
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

	BASE_TARGETOS = "win32"
	SDLOS_TARGETOS = "win32"
	SYNC_IMPLEMENTATION = "windows"

	includedirs {
		MAME_DIR .. "src/osd/windows",
		MAME_DIR .. "src/lib/winpcap",
	}

	files {
		MAME_DIR .. "src/osd/strconv.c",
		MAME_DIR .. "src/osd/windows/main.c",
		MAME_DIR .. "src/osd/windows/windir.c",
		MAME_DIR .. "src/osd/windows/winfile.c",
		MAME_DIR .. "src/osd/modules/sync/sync_windows.c",
		MAME_DIR .. "src/osd/windows/winutf8.c",
		MAME_DIR .. "src/osd/windows/winutil.c",
		MAME_DIR .. "src/osd/windows/winclip.c",
		MAME_DIR .. "src/osd/windows/winsocket.c",
		MAME_DIR .. "src/osd/windows/winptty.c",
		MAME_DIR .. "src/osd/modules/osdmodule.c",
		MAME_DIR .. "src/osd/modules/sync/work_osd.c",
		MAME_DIR .. "src/osd/modules/lib/osdlib_win32.c",
	}
