-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   sdl.lua
--
--   Rules for the building with SDL
--
---------------------------------------------------------------------------

dofile("modules.lua")


function maintargetosdoptions(_target,_subtarget)
	osdmodulestargetconf()

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
		local str = backtick("pkg-config --libs fontconfig")
		addlibfromstring(str)
		addoptionsfromstring(str)
	end

	if _OPTIONS["targetos"]=="windows" then
		if _OPTIONS["SDL_LIBVER"]=="sdl2" then
			links {
				"SDL2.dll",
			}
		else
			links {
				"SDL.dll",
			}
		end
		links {
			"psapi",
		}

		configuration { "mingw*-gcc" }
			linkoptions{
				"-municode",
			}
		configuration { "vs*" }
			flags {
				"Unicode",
			}
		configuration { "x32", "vs*" }
			libdirs {
				path.join(_OPTIONS["SDL_INSTALL_ROOT"],"lib","x86")
			}
		configuration { "x64", "vs*" }
			libdirs {
				path.join(_OPTIONS["SDL_INSTALL_ROOT"],"lib","x64")
			}
		configuration {}
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
	trigger = "MESA_INSTALL_ROOT",
	description = "link against specific GL-Library - also adds rpath to executable (overridden by USE_DISPATCH_GL)",
}

newoption {
    trigger = "SDL_INI_PATH",
    description = "Default search path for .ini files",
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
	if _OPTIONS["targetos"]=="windows" or _OPTIONS["targetos"]=="macosx" or _OPTIONS["targetos"]=="haiku" or _OPTIONS["targetos"]=="asmjs" or _OPTIONS["targetos"]=="os2" then
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
	trigger = "SDL2_MULTIAPI",
	description = "Use couriersud's multi-keyboard patch for SDL 2.1? (this API was removed prior to the 2.0 release)",
	allowed = {
		{ "0",  "Use single-keyboard API"  },
		{ "1",  "Use multi-keyboard API"   },
	},
}

if not _OPTIONS["SDL2_MULTIAPI"] then
	_OPTIONS["SDL2_MULTIAPI"] = "0"
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


BASE_TARGETOS       = "unix"
SDLOS_TARGETOS      = "unix"
SYNC_IMPLEMENTATION = "tc"
SDL_NETWORK         = ""
if _OPTIONS["targetos"]=="linux" then
	SDL_NETWORK         = "taptun"
elseif _OPTIONS["targetos"]=="openbsd" then
	SYNC_IMPLEMENTATION = "ntc"
elseif _OPTIONS["targetos"]=="netbsd" then
	SYNC_IMPLEMENTATION = "ntc"
	SDL_NETWORK         = "pcap"
elseif _OPTIONS["targetos"]=="haiku" then
	SYNC_IMPLEMENTATION = "ntc"
elseif _OPTIONS["targetos"]=="asmjs" then
	SYNC_IMPLEMENTATION = "mini"
elseif _OPTIONS["targetos"]=="windows" then
	BASE_TARGETOS       = "win32"
	SDLOS_TARGETOS      = "win32"
	SYNC_IMPLEMENTATION = "windows"
	SDL_NETWORK         = "pcap"
elseif _OPTIONS["targetos"]=="macosx" then
	SDLOS_TARGETOS      = "macosx"
	SYNC_IMPLEMENTATION = "ntc"
	SDL_NETWORK         = "pcap"
elseif _OPTIONS["targetos"]=="os2" then
	BASE_TARGETOS       = "os2"
	SDLOS_TARGETOS      = "os2"
	SYNC_IMPLEMENTATION = "os2"
end

if _OPTIONS["SDL_LIBVER"]=="sdl" then
	USE_BGFX = 0
end

if BASE_TARGETOS=="unix" then
	if _OPTIONS["targetos"]=="macosx" then
		local os_version = str_to_version(backtick("sw_vers -productVersion"))

		links {
			"Cocoa.framework",
		}
		linkoptions {
			"-framework QuartzCore",
			"-framework OpenGL",
		}
		if os_version>=101100 then
			linkoptions {
				"-weak_framework Metal",
			}
		end
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
			local str = backtick(sdlconfigcmd() .. " --libs | sed 's/-lSDLmain//'")
			addlibfromstring(str)
			addoptionsfromstring(str)
		end
	else
		if _OPTIONS["NO_X11"]=="1" then
			_OPTIONS["USE_QTDEBUG"] = "0"
			USE_BGFX = 0
		else
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
		local str = backtick(sdlconfigcmd() .. " --libs")
		addlibfromstring(str)
		addoptionsfromstring(str)
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
	local str = backtick(sdlconfigcmd() .. " --libs")
	addlibfromstring(str)
	addoptionsfromstring(str)
	links {
		"pthread"
	}
end

project ("qtdbg_" .. _OPTIONS["osd"])
	uuid (os.uuid("qtdbg_" .. _OPTIONS["osd"]))
	kind (LIBTYPE)

	dofile("sdl_cfg.lua")
	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices", -- accessing imagedev from debugger
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "src/osd/modules/render",
		MAME_DIR .. "3rdparty",
	}
	configuration { "linux-*" }
		buildoptions {
			"-fPIC",
		}
	configuration { }

	qtdebuggerbuild()

project ("osd_" .. _OPTIONS["osd"])
	targetsubdir(_OPTIONS["target"] .."_" .._OPTIONS["subtarget"])
	uuid (os.uuid("osd_" .. _OPTIONS["osd"]))
	kind (LIBTYPE)

	dofile("sdl_cfg.lua")
	osdmodulesbuild()

	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices", -- accessing imagedev from debugger
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "src/osd/modules/render",
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "src/osd/sdl",
	}

	if _OPTIONS["targetos"]=="windows" then
		files {
			MAME_DIR .. "src/osd/sdl/main.cpp",
		}
	end

	if _OPTIONS["targetos"]=="macosx" then
		files {
			MAME_DIR .. "src/osd/modules/debugger/debugosx.mm",
			MAME_DIR .. "src/osd/modules/debugger/osx/breakpointsview.mm",
			MAME_DIR .. "src/osd/modules/debugger/osx/breakpointsview.h",
			MAME_DIR .. "src/osd/modules/debugger/osx/consoleview.mm",
			MAME_DIR .. "src/osd/modules/debugger/osx/consoleview.h",
			MAME_DIR .. "src/osd/modules/debugger/osx/debugcommandhistory.mm",
			MAME_DIR .. "src/osd/modules/debugger/osx/debugcommandhistory.h",
			MAME_DIR .. "src/osd/modules/debugger/osx/debugconsole.mm",
			MAME_DIR .. "src/osd/modules/debugger/osx/debugconsole.h",
			MAME_DIR .. "src/osd/modules/debugger/osx/debugview.mm",
			MAME_DIR .. "src/osd/modules/debugger/osx/debugview.h",
			MAME_DIR .. "src/osd/modules/debugger/osx/debugwindowhandler.mm",
			MAME_DIR .. "src/osd/modules/debugger/osx/debugwindowhandler.h",
			MAME_DIR .. "src/osd/modules/debugger/osx/deviceinfoviewer.mm",
			MAME_DIR .. "src/osd/modules/debugger/osx/deviceinfoviewer.h",
			MAME_DIR .. "src/osd/modules/debugger/osx/devicesviewer.mm",
			MAME_DIR .. "src/osd/modules/debugger/osx/devicesviewer.h",
			MAME_DIR .. "src/osd/modules/debugger/osx/disassemblyview.mm",
			MAME_DIR .. "src/osd/modules/debugger/osx/disassemblyviewer.mm",
			MAME_DIR .. "src/osd/modules/debugger/osx/disassemblyviewer.h",
			MAME_DIR .. "src/osd/modules/debugger/osx/errorlogview.mm",
			MAME_DIR .. "src/osd/modules/debugger/osx/errorlogview.h",
			MAME_DIR .. "src/osd/modules/debugger/osx/disassemblyview.h",
			MAME_DIR .. "src/osd/modules/debugger/osx/errorlogviewer.mm",
			MAME_DIR .. "src/osd/modules/debugger/osx/errorlogviewer.h",
			MAME_DIR .. "src/osd/modules/debugger/osx/memoryview.mm",
			MAME_DIR .. "src/osd/modules/debugger/osx/memoryview.h",
			MAME_DIR .. "src/osd/modules/debugger/osx/memoryviewer.mm",
			MAME_DIR .. "src/osd/modules/debugger/osx/memoryviewer.h",
			MAME_DIR .. "src/osd/modules/debugger/osx/pointsviewer.mm",
			MAME_DIR .. "src/osd/modules/debugger/osx/pointsviewer.h",
			MAME_DIR .. "src/osd/modules/debugger/osx/registersview.mm",
			MAME_DIR .. "src/osd/modules/debugger/osx/registersview.h",
			MAME_DIR .. "src/osd/modules/debugger/osx/watchpointsview.mm",
			MAME_DIR .. "src/osd/modules/debugger/osx/watchpointsview.h",
			MAME_DIR .. "src/osd/modules/debugger/osx/debugosx.h",
		}
		if _OPTIONS["SDL_LIBVER"]=="sdl" then
			-- SDLMain_tmpl isn't necessary for SDL2
			files {
				MAME_DIR .. "src/osd/sdl/SDLMain_tmpl.mm",
				MAME_DIR .. "src/osd/sdl/SDLMain_tmpl.h",
			}
		end
	end

	files {
		MAME_DIR .. "src/osd/sdl/osdsdl.h",
		MAME_DIR .. "src/osd/sdl/sdlinc.h",
		MAME_DIR .. "src/osd/sdl/sdlprefix.h",
		MAME_DIR .. "src/osd/sdl/sdlmain.cpp",
		MAME_DIR .. "src/osd/osdepend.h",
		MAME_DIR .. "src/osd/sdl/input.cpp",
		MAME_DIR .. "src/osd/sdl/input.h",
		MAME_DIR .. "src/osd/sdl/video.cpp",
		MAME_DIR .. "src/osd/sdl/video.h",
		MAME_DIR .. "src/osd/sdl/window.cpp",
		MAME_DIR .. "src/osd/sdl/window.h",
		MAME_DIR .. "src/osd/modules/osdwindow.h",
		MAME_DIR .. "src/osd/sdl/output.cpp",
		MAME_DIR .. "src/osd/sdl/watchdog.cpp",
		MAME_DIR .. "src/osd/sdl/watchdog.h",
		MAME_DIR .. "src/osd/modules/render/drawsdl.cpp",
	}
	if _OPTIONS["SDL_LIBVER"]=="sdl2" then
		files {
			MAME_DIR .. "src/osd/modules/render/draw13.cpp",
			MAME_DIR .. "src/osd/modules/render/blit13.h",
		}
	end


project ("ocore_" .. _OPTIONS["osd"])
	targetsubdir(_OPTIONS["target"] .."_" .. _OPTIONS["subtarget"])
	uuid (os.uuid("ocore_" .. _OPTIONS["osd"]))
	kind (LIBTYPE)

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
		MAME_DIR .. "src/osd/osdcore.cpp",
		MAME_DIR .. "src/osd/osdcore.h",
		MAME_DIR .. "src/osd/strconv.cpp",
		MAME_DIR .. "src/osd/strconv.h",
		MAME_DIR .. "src/osd/sdl/sdldir.cpp",
		MAME_DIR .. "src/osd/sdl/sdlfile.cpp",
		MAME_DIR .. "src/osd/sdl/sdlfile.h",
		MAME_DIR .. "src/osd/sdl/sdlptty_" .. BASE_TARGETOS ..".cpp",
		MAME_DIR .. "src/osd/sdl/sdlsocket.cpp",
		MAME_DIR .. "src/osd/sdl/sdlos_" .. SDLOS_TARGETOS .. ".cpp",
		MAME_DIR .. "src/osd/modules/osdmodule.cpp",
		MAME_DIR .. "src/osd/modules/osdmodule.h",
		MAME_DIR .. "src/osd/modules/lib/osdlib_" .. SDLOS_TARGETOS .. ".cpp",
		MAME_DIR .. "src/osd/modules/lib/osdlib.h",
		MAME_DIR .. "src/osd/modules/sync/sync_" .. SYNC_IMPLEMENTATION .. ".cpp",
		MAME_DIR .. "src/osd/modules/sync/osdsync.h",
	}

	if _OPTIONS["NOASM"]=="1" then
		files {
			MAME_DIR .. "src/osd/modules/sync/work_mini.cpp",
		}
	else
		files {
			MAME_DIR .. "src/osd/modules/sync/work_osd.cpp",
		}
	end

	if _OPTIONS["targetos"]=="macosx" then
		files {
			MAME_DIR .. "src/osd/sdl/osxutils.h",
			MAME_DIR .. "src/osd/sdl/osxutils.mm",
		}
	end


--------------------------------------------------
-- testkeys
--------------------------------------------------

if _OPTIONS["with-tools"] then
	project("testkeys")
		uuid ("744cec21-c3b6-4d69-93cb-6811fed0ffe3")
		kind "ConsoleApp"

		flags {
			"Symbols", -- always include minimum symbols for executables
		}

		dofile("sdl_cfg.lua")

		includedirs {
			MAME_DIR .. "src/osd",
			MAME_DIR .. "src/lib/util",
		}

		if _OPTIONS["SEPARATE_BIN"]~="1" then
			targetdir(MAME_DIR)
		end

		links {
			"utils",
			"ocore_" .. _OPTIONS["osd"],
		}

		files {
			MAME_DIR .. "src/osd/sdl/testkeys.cpp",
		}

		if _OPTIONS["targetos"] == "windows" then
			if _OPTIONS["SDL_LIBVER"] == "sdl2" then
				links {
					"SDL2.dll",
				}
			else
				links {
					"SDL.dll",
				}
			end
			links {
				"psapi",
			}
			linkoptions{
				"-municode",
			}
			files {
				MAME_DIR .. "src/osd/sdl/main.cpp",
			}
		elseif _OPTIONS["targetos"] == "macosx" and _OPTIONS["SDL_LIBVER"] == "sdl" then
			-- SDLMain_tmpl isn't necessary for SDL2
			files {
				MAME_DIR .. "src/osd/sdl/SDLMain_tmpl.mm",
			}
		end
end


--------------------------------------------------
-- aueffectutil
--------------------------------------------------

if _OPTIONS["targetos"] == "macosx" and _OPTIONS["with-tools"] then
	project("aueffectutil")
		uuid ("3db8316d-fad7-4f5b-b46a-99373c91550e")
		kind "ConsoleApp"

		flags {
			"Symbols", -- always include minimum symbols for executables
		}

		dofile("sdl_cfg.lua")

		if _OPTIONS["SEPARATE_BIN"]~="1" then
			targetdir(MAME_DIR)
		end

		linkoptions {
			"-sectcreate __TEXT __info_plist " .. MAME_DIR .. "src/osd/sdl/aueffectutil-Info.plist",
		}

		dependency {
			{ "aueffectutil",  MAME_DIR .. "src/osd/sdl/aueffectutil-Info.plist", true  },
		}

		links {
			"AudioUnit.framework",
			"AudioToolbox.framework",
			"CoreAudio.framework",
			"CoreAudioKit.framework",
			"CoreServices.framework",
		}

		files {
			MAME_DIR .. "src/osd/sdl/aueffectutil.mm",
		}
end
