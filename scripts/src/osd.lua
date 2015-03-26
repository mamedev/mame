project ("osd_" .. _OPTIONS["osd"])
	uuid (os.uuid("osd_" .. _OPTIONS["osd"]))
	kind "StaticLib"

	removeflags {
		"SingleOutputDir",
	}
	
	options {
		"ForceCPP",
	}

	dofile("osd_cfg.lua")
	
	if _OPTIONS["osd"]=="windows" then
		defines {
			"DIRECTINPUT_VERSION=0x0800",
			"DIRECT3D_VERSION=0x0900",
		}
	end

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
	if _OPTIONS["osd"]=="osdmini" then
		includedirs {
			MAME_DIR .. "src/osd/sdl",
		}
		files {
			MAME_DIR .. "src/osd/osdmini/minimain.c",
		}
	end
	if _OPTIONS["osd"]=="sdl" then
		includedirs {
			MAME_DIR .. "src/osd/sdl",
		}
		if _OPTIONS["targetos"]=="windows" then
			files {
				MAME_DIR .. "src/osd/sdl/main.c",
			}
		end
		if _OPTIONS["targetos"]=="macosx" then
			files {
				--MAME_DIR .. "src/osd/sdl/SDLMain_tmpl.m",
				MAME_DIR .. "src/osd/modules/debugger/*.m",
				MAME_DIR .. "src/osd/modules/debugger/osx/*.m",
				MAME_DIR .. "src/osd/modules/debugger/osx/*.h",
			}
		end
		files {
			MAME_DIR .. "src/osd/sdl/sdlmain.*",
			MAME_DIR .. "src/osd/sdl/input.*",
			MAME_DIR .. "src/osd/sdl/video.*",
			MAME_DIR .. "src/osd/sdl/window.*",
			MAME_DIR .. "src/osd/sdl/output.*",
			MAME_DIR .. "src/osd/sdl/watchdog.*",
			MAME_DIR .. "src/osd/modules/render/drawsdl.*",
			--ifeq ($(SDL_LIBVER),sdl2)
			MAME_DIR .. "src/osd/modules/render/draw13.*",
			--endif
			MAME_DIR .. "src/osd/modules/debugger/none.*",
			MAME_DIR .. "src/osd/modules/debugger/debugint.*",
			MAME_DIR .. "src/osd/modules/debugger/debugwin.*",
			MAME_DIR .. "src/osd/modules/debugger/debugqt.*",

			MAME_DIR .. "src/osd/modules/render/drawogl.*",
			MAME_DIR .. "src/osd/modules/opengl/gl_shader_tool.*",
			MAME_DIR .. "src/osd/modules/opengl/gl_shader_mgr.*",
		}
		if not (_OPTIONS["targetos"]=="macosx") then
		files {
			MAME_DIR .. "src/osd/modules/debugger/qt/*.*",
			GEN_DIR  .. "osd/modules/debugger/qt/*.*",
		}
		end

		if (USE_BGFX == 1) then
		files {
			MAME_DIR .. "src/osd/modules/render/drawbgfx.c",
		}
		end
	end
	if _OPTIONS["osd"]=="windows" then
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
			}
	end
	files {
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

	dofile("osd_cfg.lua")
	
	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
	}
	if _OPTIONS["targetos"]=="linux" then
		BASE_TARGETOS = "unix"
		SDLOS_TARGETOS = "unix"
		SYNC_IMPLEMENTATION = "tc"
	end
	if _OPTIONS["targetos"]=="windows" then
		BASE_TARGETOS = "win32"
		SDLOS_TARGETOS = "win32"
		SYNC_IMPLEMENTATION = "windows"
	end
	if _OPTIONS["targetos"]=="macosx" then
		BASE_TARGETOS = "unix"
		SDLOS_TARGETOS = "macosx"
		SYNC_IMPLEMENTATION = "ntc"
	end
	files {
		MAME_DIR .. "src/osd/modules/osdmodule.*",
	}
	if _OPTIONS["osd"]=="sdl" then		
		includedirs {
			MAME_DIR .. "src/osd/sdl",
		}
		if _OPTIONS["targetos"]=="macosx" then
			files {
				MAME_DIR .. "src/osd/sdl/osxutils.m",
			}
		end
		files {
			MAME_DIR .. "src/osd/sdl/strconv.*",
			MAME_DIR .. "src/osd/sdl/sdldir.*",
			MAME_DIR .. "src/osd/sdl/sdlfile.*",
			MAME_DIR .. "src/osd/sdl/sdlptty_" .. BASE_TARGETOS ..".*",
			MAME_DIR .. "src/osd/sdl/sdlsocket.*",
			MAME_DIR .. "src/osd/sdl/sdlos_" .. SDLOS_TARGETOS .. ".*",
			MAME_DIR .. "src/osd/modules/lib/osdlib_" .. SDLOS_TARGETOS .. ".*",
			MAME_DIR .. "src/osd/modules/sync/sync_" .. SYNC_IMPLEMENTATION .. ".*",
			--ifdef NOASM
			--MAME_DIR .. "src/osd/modules/sync/work_mini.*",
			--else
			MAME_DIR .. "src/osd/modules/sync/work_osd.*",
		}
		if _OPTIONS["targetos"]=="macosx" then
			files {
				MAME_DIR .. "src/osd/sdl/osxutils.m",
			}
		end
	end
	if _OPTIONS["osd"]=="windows" then
		includedirs {
			MAME_DIR .. "src/osd/windows",
			MAME_DIR .. "src/lib/winpcap",
		}
		files {
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
	end
	if _OPTIONS["osd"]=="osdmini" then
		files {
			MAME_DIR .. "src/osd/osdmini/minidir.*",
			MAME_DIR .. "src/osd/osdmini/minifile.*",
			MAME_DIR .. "src/osd/osdmini/minimisc.*",
			MAME_DIR .. "src/osd/osdmini/minisync.*",
			MAME_DIR .. "src/osd/osdmini/minitime.*",
			MAME_DIR .. "src/osd/modules/sync/work_mini.*",
		}
	end
