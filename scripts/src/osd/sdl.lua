function maintargetosdoptions(_target)
	if _OPTIONS["USE_DISPATCH_GL"]~="1" and _OPTIONS["MESA_INSTALL_ROOT"] then
		libdirs {
			path.join(_OPTIONS["MESA_INSTALL_ROOT"],"lib"),
		}
		linkoptions {
			"-Wl,-rpath=" .. path.join(_OPTIONS["MESA_INSTALL_ROOT"],"lib"),
		}
	end

	if _OPTIONS["NO_X11"]~="1" then
		links {
			"X11",
			"Xinerama",
		}
	end

	if _OPTIONS["NO_USE_XINPUT"]~="1" then
		links {
			"Xext",
			"Xi",
		}
	end

	if BASE_TARGETOS=="unix" and _OPTIONS["targetos"]~="macosx" then
		if _OPTIONS["SDL_LIBVER"]=="sdl2" then
			links {
				"SDL2_ttf",
			}
		else
			links {
				"SDL_ttf",
			}
		end
		if _OPTIONS["NO_OPENGL"]~="1" and _OPTIONS["USE_DISPATCH_GL"]~="1" then
			links {
				"GL"
			}
		end
		linkoptions {
			string.gsub(os.outputof("pkg-config --libs fontconfig"), '[\r\n]+', ' '),
		}
	end

	if _OPTIONS["targetos"]=="windows" then
		if _OPTIONS["NO_OPENGL"]~="1" and _OPTIONS["USE_DISPATCH_GL"]~="1" then
			links {
				"opengl32"
			}
		end
		configuration { "mingw*" }
			linkoptions{
				"-municode",
			}
		configuration { "x32", "vs*" }
			libdirs {
				path.join(_OPTIONS["SDL_INSTALL_ROOT"],"lib","x86")
			}
		configuration { "x64", "vs*" }
			libdirs {
				path.join(_OPTIONS["SDL_INSTALL_ROOT"],"lib","x64")
			}
		configuration { "vs*" }	
			links {
				"SDL2",
			}
		configuration {}

		if (USE_QT == 1) then
			linkoptions{
				"-L$(shell qmake -query QT_INSTALL_LIBS)",
			}
			links {
				"qtmain",
				"QtGui4",
				"QtCore4",
			}
		end
	elseif _OPTIONS["targetos"]=="linux" then
		if USE_QT == 1 then
			linkoptions {
				"$(shell pkg-config --libs QtGui)",
			}
			links {
				"QtGui",
				"QtCore",
			}
		end
		if _OPTIONS["NO_USE_MIDI"]~="1" then
			linkoptions {
				string.gsub(os.outputof("pkg-config --libs alsa"), '[\r\n]+', ' '),
			}
		end
	elseif _OPTIONS["targetos"]=="macosx" then
		if _OPTIONS["NO_USE_MIDI"]~="1" then
			links {
				"CoreAudio.framework",
				"CoreMIDI.framework",
			}
		end
	elseif _OPTIONS["targetos"]=="haiku" then
		links {
			"network",
			"bsd",
		}
	end
	
	configuration { "mingw*" or "vs*" }
		targetprefix "sdl"

	configuration { }
end


function sdlconfigcmd()
	if not _OPTIONS["SDL_INSTALL_ROOT"] then
		return _OPTIONS["SDL_LIBVER"] .. "-config"
	else
		return path.join(_OPTIONS["SDL_INSTALL_ROOT"],"bin",_OPTIONS["SDL_LIBVER"]) .. "-config"
	end
end


newoption {
	trigger = "NO_OPENGL",
	description = "Disable use of OpenGL",
	allowed = {
		{ "0",  "Enable OpenGL"  },
		{ "1",  "Disable OpenGL" },
	},
}

if not _OPTIONS["NO_OPENGL"] then
	if _OPTIONS["targetos"]=="os2" then
		_OPTIONS["NO_OPENGL"] = "1"
	else
		_OPTIONS["NO_OPENGL"] = "0"
	end
end

newoption {
	trigger = "USE_DISPATCH_GL",
	description = "Use GL-dispatching (takes precedence over MESA_INSTALL_ROOT)",
	allowed = {
		{ "0",  "Link to OpenGL library"  },
		{ "1",  "Use GL-dispatching"      },
	},
}

if not _OPTIONS["USE_DISPATCH_GL"] then
	if USE_BGFX == 1 then
		_OPTIONS["USE_DISPATCH_GL"] = "0"
	else
		_OPTIONS["USE_DISPATCH_GL"] = "1"
	end
end

newoption {
	trigger = "MESA_INSTALL_ROOT",
	description = "link against specific GL-Library - also adds rpath to executable",
}

newoption {
	trigger = "NO_X11",
	description = "Disable use of X11",
	allowed = {
		{ "0",  "Enable X11"  },
		{ "1",  "Disable X11" },
	},
}

if not _OPTIONS["NO_X11"] then
	if _OPTIONS["targetos"]=="windows" or _OPTIONS["targetos"]=="macosx" or _OPTIONS["targetos"]=="haiku" or _OPTIONS["targetos"]=="emscripten" or _OPTIONS["targetos"]=="os2" then
		_OPTIONS["NO_X11"] = "1"
	else
		_OPTIONS["NO_X11"] = "0"
	end
end

newoption {
	trigger = "NO_USE_XINPUT",
	description = "Disable use of Xinput",
	allowed = {
		{ "0",  "Enable Xinput"  },
		{ "1",  "Disable Xinput" },
	},
}

if not _OPTIONS["NO_USE_XINPUT"] then
	_OPTIONS["NO_USE_XINPUT"] = "1"
end

newoption {
	trigger = "NO_USE_MIDI",
	description = "Disable MIDI I/O",
	allowed = {
		{ "0",  "Enable MIDI"  },
		{ "1",  "Disable MIDI" },
	},
}

if not _OPTIONS["NO_USE_MIDI"] then
	if _OPTIONS["targetos"]=="freebsd" or _OPTIONS["targetos"]=="openbsd" or _OPTIONS["targetos"]=="netbsd" or _OPTIONS["targetos"]=="solaris" or _OPTIONS["targetos"]=="haiku" or _OPTIONS["targetos"] == "emscripten" or _OPTIONS["targetos"] == "os2" then
		_OPTIONS["NO_USE_MIDI"] = "1"
	else
		_OPTIONS["NO_USE_MIDI"] = "0"
	end
end

newoption {
	trigger = "SDL_LIBVER",
	description = "Choose SDL version",
	allowed = {
		{ "sdl",   "SDL"   },
		{ "sdl2",  "SDL 2" },
	},
}

if not _OPTIONS["SDL_LIBVER"] then
	if _OPTIONS["targetos"]=="os2" then
		_OPTIONS["SDL_LIBVER"] = "sdl"
	else
		_OPTIONS["SDL_LIBVER"] = "sdl2"
	end
end

newoption {
	trigger = "SDL_INSTALL_ROOT",
	description = "Equivalent to the ./configure --prefix=<path>",
}

newoption {
	trigger = "SDL_FRAMEWORK_PATH",
	description = "Location of SDL framework for custom OS X installations",
}

if not _OPTIONS["SDL_FRAMEWORK_PATH"] then
	_OPTIONS["SDL_FRAMEWORK_PATH"] = "/Library/Frameworks/"
end

newoption {
	trigger = "MACOSX_USE_LIBSDL",
	description = "Use SDL library on OS (rather than framework)",
	allowed = {
		{ "0",  "Use framework"  },
		{ "1",  "Use library" },
	},
}

if not _OPTIONS["MACOSX_USE_LIBSDL"] then
	_OPTIONS["MACOSX_USE_LIBSDL"] = "0"
end


BASE_TARGETOS = "unix"
SDLOS_TARGETOS = "unix"
SYNC_IMPLEMENTATION = "tc"
if _OPTIONS["targetos"]=="openbsd" then
	SYNC_IMPLEMENTATION = "ntc"
elseif _OPTIONS["targetos"]=="netbsd" then
	SYNC_IMPLEMENTATION = "ntc"
elseif _OPTIONS["targetos"]=="haiku" then
	SYNC_IMPLEMENTATION = "ntc"
elseif _OPTIONS["targetos"]=="emscripten" then
	SYNC_IMPLEMENTATION = "mini"
elseif _OPTIONS["targetos"]=="windows" then
	BASE_TARGETOS = "win32"
	SDLOS_TARGETOS = "win32"
	SYNC_IMPLEMENTATION = "windows"
elseif _OPTIONS["targetos"]=="macosx" then
	SDLOS_TARGETOS = "macosx"
	SYNC_IMPLEMENTATION = "ntc"
elseif _OPTIONS["targetos"]=="os2" then
	BASE_TARGETOS = "os2"
	SDLOS_TARGETOS = "os2"
	SYNC_IMPLEMENTATION = "os2"
end

if _OPTIONS["NO_X11"]~="1" then
	libdirs {
		"/usr/X11/lib",
		"/usr/X11R6/lib",
		"/usr/openwin/lib",
	}
	if _OPTIONS["SDL_LIBVER"]=="sdl" then
		links {
			"X11",
		}
	end
end

if BASE_TARGETOS=="unix" then
	if _OPTIONS["targetos"]=="macosx" then
		links {
			"Cocoa.framework",
			"OpenGL.framework",
		}
		if _OPTIONS["MACOSX_USE_LIBSDL"]~="1" then
			linkoptions {
				"-F" .. _OPTIONS["SDL_FRAMEWORK_PATH"],
			}
			if _OPTIONS["SDL_LIBVER"]=="sdl2" then
				links {
					"SDL2.framework",
				}
			else
				links {
					"SDL.framework",
				}
			end
		else
			linkoptions {
				string.gsub(os.outputof(sdlconfigcmd() .. " --libs | sed 's/-lSDLmain//'"), '[\r\n]+', ' '),
			}
		end
	else
		linkoptions {
			string.gsub(os.outputof(sdlconfigcmd() .. " --libs"), '[\r\n]+', ' '),
		}
		if _OPTIONS["targetos"]~="haiku" then
			links {
				"m",
				"pthread",
			}
			if _OPTIONS["targetos"]=="solaris" then
				links {
					"socket",
					"nsl",
				}
			else
				links {
					"util",
				}
			end
		end
	end
elseif BASE_TARGETOS=="os2" then
	linkoptions {
		string.gsub(os.outputof(sdlconfigcmd() .. " --libs"), '[\r\n]+', ' '),
	}
	links {
		"pthread"
	}
end

configuration { "mingw*" }
		linkoptions {
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
		if _OPTIONS["SDL_LIBVER"]=="sdl" then
			-- SDLMain_tmpl isn't necessary for SDL2
			files {
				MAME_DIR .. "src/osd/sdl/SDLMain_tmpl.m",
			}
		end
	end

	files {
		MAME_DIR .. "src/osd/sdl/sdlmain.c",
		MAME_DIR .. "src/osd/sdl/input.c",
		MAME_DIR .. "src/osd/sdl/video.c",
		MAME_DIR .. "src/osd/sdl/window.c",
		MAME_DIR .. "src/osd/sdl/output.c",
		MAME_DIR .. "src/osd/sdl/watchdog.c",
		MAME_DIR .. "src/osd/modules/lib/osdobj_common.c",
		MAME_DIR .. "src/osd/modules/render/drawsdl.c",
		MAME_DIR .. "src/osd/modules/debugger/none.c",
		MAME_DIR .. "src/osd/modules/debugger/debugint.c",
		MAME_DIR .. "src/osd/modules/debugger/debugwin.c",
		MAME_DIR .. "src/osd/modules/debugger/debugqt.c",
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
	if _OPTIONS["NO_OPENGL"]~="1" then
		files {
			MAME_DIR .. "src/osd/modules/render/drawogl.c",
			MAME_DIR .. "src/osd/modules/opengl/gl_shader_tool.c",
			MAME_DIR .. "src/osd/modules/opengl/gl_shader_mgr.c",
		}
	end
	if _OPTIONS["SDL_LIBVER"]=="sdl2" then
		files {
			MAME_DIR .. "src/osd/modules/render/draw13.c",
		}
	end

	if USE_QT == 1 then
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

	if USE_BGFX == 1 then
		files {
			MAME_DIR .. "src/osd/modules/render/drawbgfx.c",
		}
	end


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


--------------------------------------------------
-- testkeys
--------------------------------------------------

if _OPTIONS["with-tools"] then
	project("testkeys")
		uuid ("744cec21-c3b6-4d69-93cb-6811fed0ffe3")
		kind "ConsoleApp"

		options {
			"ForceCPP",
		}

		dofile("sdl_cfg.lua")

		includedirs {
			MAME_DIR .. "src/lib/util",
		}
		targetdir(MAME_DIR)

		links {
			"utils",
			"ocore_" .. _OPTIONS["osd"],
		}

		includeosd()

		files {
			MAME_DIR .. "src/osd/sdl/testkeys.c",
		}

		if _OPTIONS["targetos"]=="windows" then
			linkoptions{
				"-municode",
			}
			files {
				MAME_DIR .. "src/osd/sdl/main.c",
			}
		elseif _OPTIONS["targetos"]=="macosx" and _OPTIONS["SDL_LIBVER"]=="sdl" then
			-- SDLMain_tmpl isn't necessary for SDL2
			files {
				MAME_DIR .. "src/osd/sdl/SDLMain_tmpl.m",
			}
		end
end
