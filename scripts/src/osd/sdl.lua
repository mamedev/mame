function maintargetosdoptions(_target)
	if _OPTIONS["targetos"]=="windows" then
		linkoptions{
			"-L$(shell qmake -query QT_INSTALL_LIBS)",
		}

		links {
			"qtmain",
			"QtGui4",
			"QtCore4",
		}
	end

	if _OPTIONS["targetos"]=="linux" then
		links {
			'QtGui',
			'QtCore',
		}

		linkoptions {
			'$(shell pkg-config --libs QtGui)',
		}
	end

	configuration { "mingw*" or "vs*" }
		targetprefix "sdl"

	configuration { }
end


if _OPTIONS["targetos"]=="windows" then
	linkoptions {
		"-municode",
	}
end


configuration { "mingw*" }
		linkoptions {
			"-Wl,--allow-multiple-definition",
			"-static"
		}

configuration { }


project ("osd_" .. _OPTIONS["osd"])
	uuid (os.uuid("osd_" .. _OPTIONS["osd"]))
	kind "StaticLib"

	removeflags {
		"SingleOutputDir",
	}
	
	options {
		"ForceCPP",
	}

	dofile("sdl_cfg.lua")
	
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
			MAME_DIR .. "src/osd/modules/debugger/debugosx.m",
			MAME_DIR .. "src/osd/modules/debugger/osx/breakpointsview.m",
			MAME_DIR .. "src/osd/modules/debugger/osx/consoleview.m",
			MAME_DIR .. "src/osd/modules/debugger/osx/debugcommandhistory.m",
			MAME_DIR .. "src/osd/modules/debugger/osx/debugconsole.m",
			MAME_DIR .. "src/osd/modules/debugger/osx/debugview.m",
			MAME_DIR .. "src/osd/modules/debugger/osx/debugwindowhandler.m",
			MAME_DIR .. "src/osd/modules/debugger/osx/deviceinfoviewer.m",
			MAME_DIR .. "src/osd/modules/debugger/osx/devicesviewer.m",
			MAME_DIR .. "src/osd/modules/debugger/osx/disassemblyview.m",
			MAME_DIR .. "src/osd/modules/debugger/osx/disassemblyviewer.m",
			MAME_DIR .. "src/osd/modules/debugger/osx/errorlogview.m",
			MAME_DIR .. "src/osd/modules/debugger/osx/errorlogviewer.m",
			MAME_DIR .. "src/osd/modules/debugger/osx/memoryview.m",
			MAME_DIR .. "src/osd/modules/debugger/osx/memoryviewer.m",
			MAME_DIR .. "src/osd/modules/debugger/osx/pointsviewer.m",
			MAME_DIR .. "src/osd/modules/debugger/osx/registersview.m",
			MAME_DIR .. "src/osd/modules/debugger/osx/watchpointsview.m",
		}
	end

	files {
		MAME_DIR .. "src/osd/sdl/sdlmain.c",
		MAME_DIR .. "src/osd/sdl/input.c",
		MAME_DIR .. "src/osd/sdl/video.c",
		MAME_DIR .. "src/osd/sdl/window.c",
		MAME_DIR .. "src/osd/sdl/output.c",
		MAME_DIR .. "src/osd/sdl/watchdog.c",
		MAME_DIR .. "src/osd/modules/render/drawsdl.c",
		--ifeq ($(SDL_LIBVER),sdl2)
		MAME_DIR .. "src/osd/modules/render/draw13.c",
		--endif
		MAME_DIR .. "src/osd/modules/debugger/none.c",
		MAME_DIR .. "src/osd/modules/debugger/debugint.c",
		MAME_DIR .. "src/osd/modules/debugger/debugwin.c",
		MAME_DIR .. "src/osd/modules/debugger/debugqt.c",
		MAME_DIR .. "src/osd/modules/render/drawogl.c",
		MAME_DIR .. "src/osd/modules/opengl/gl_shader_tool.c",
		MAME_DIR .. "src/osd/modules/opengl/gl_shader_mgr.c",
	}

	if not (_OPTIONS["targetos"]=="macosx") then
		files {
			MAME_DIR .. "src/osd/modules/debugger/qt/debuggerview.c",
			MAME_DIR .. "src/osd/modules/debugger/qt/windowqt.c",
			MAME_DIR .. "src/osd/modules/debugger/qt/logwindow.c",
			MAME_DIR .. "src/osd/modules/debugger/qt/dasmwindow.c",
			MAME_DIR .. "src/osd/modules/debugger/qt/mainwindow.c",
			MAME_DIR .. "src/osd/modules/debugger/qt/memorywindow.c",
			MAME_DIR .. "src/osd/modules/debugger/qt/breakpointswindow.c",
			MAME_DIR .. "src/osd/modules/debugger/qt/deviceswindow.c",
			MAME_DIR .. "src/osd/modules/debugger/qt/deviceinformationwindow.c",

			GEN_DIR  .. "osd/modules/debugger/qt/debuggerview.moc.c",
			GEN_DIR  .. "osd/modules/debugger/qt/windowqt.moc.c",
			GEN_DIR  .. "osd/modules/debugger/qt/logwindow.moc.c",
			GEN_DIR  .. "osd/modules/debugger/qt/dasmwindow.moc.c",
			GEN_DIR  .. "osd/modules/debugger/qt/mainwindow.moc.c",
			GEN_DIR  .. "osd/modules/debugger/qt/memorywindow.moc.c",
			GEN_DIR  .. "osd/modules/debugger/qt/breakpointswindow.moc.c",
			GEN_DIR  .. "osd/modules/debugger/qt/deviceswindow.moc.c",
			GEN_DIR  .. "osd/modules/debugger/qt/deviceinformationwindow.moc.c",
		}
	end

	if (USE_BGFX == 1) then
		files {
			MAME_DIR .. "src/osd/modules/render/drawbgfx.c",
		}
	end

	files {
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

	dofile("sdl_cfg.lua")
	
	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "src/osd/sdl",
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
		MAME_DIR .. "src/osd/strconv.c",
		MAME_DIR .. "src/osd/sdl/sdldir.c",
		MAME_DIR .. "src/osd/sdl/sdlfile.c",
		MAME_DIR .. "src/osd/sdl/sdlptty_" .. BASE_TARGETOS ..".c",
		MAME_DIR .. "src/osd/sdl/sdlsocket.c",
		MAME_DIR .. "src/osd/sdl/sdlos_" .. SDLOS_TARGETOS .. ".c",
		MAME_DIR .. "src/osd/modules/osdmodule.c",
		MAME_DIR .. "src/osd/modules/lib/osdlib_" .. SDLOS_TARGETOS .. ".c",
		MAME_DIR .. "src/osd/modules/sync/sync_" .. SYNC_IMPLEMENTATION .. ".c",
		--ifdef NOASM
		--MAME_DIR .. "src/osd/modules/sync/work_mini.c",
		--else
		MAME_DIR .. "src/osd/modules/sync/work_osd.c",
	}

	if _OPTIONS["targetos"]=="macosx" then
		files {
			MAME_DIR .. "src/osd/sdl/osxutils.m",
		}
	end
