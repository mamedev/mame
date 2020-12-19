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
	else
		if _OPTIONS["targetos"]=="linux" or _OPTIONS["targetos"]=="netbsd" or _OPTIONS["targetos"]=="openbsd" then
			links {
				"EGL",
			}
		end
	end

	if _OPTIONS["NO_USE_XINPUT"]~="1" then
		links {
			"Xext",
			"Xi",
		}
	end

	if BASE_TARGETOS=="unix" and _OPTIONS["targetos"]~="macosx" and _OPTIONS["targetos"]~="android" and _OPTIONS["targetos"]~="asmjs" then
		links {
			"SDL2_ttf",
		}
		local str = backtick(pkgconfigcmd() .. " --libs fontconfig")
		addlibfromstring(str)
		addoptionsfromstring(str)
	end

	if _OPTIONS["targetos"]=="windows" then
		if _OPTIONS["with-bundled-sdl2"]~=nil then
			configuration { "mingw*"}
				links {
					"SDL2",
					"imm32",
					"version",
					"ole32",
					"oleaut32",
				}
			configuration { "vs*" }
				links {
					"SDL2",
					"imm32",
					"version",
				}
			configuration { }
		else
			if _OPTIONS["USE_LIBSDL"]~="1" then
				configuration { "mingw*"}
					links {
						"SDL2main",
						"SDL2",
					}
				configuration { "vs*" }
					links {
						"SDL2",
						"imm32",
						"version",
					}
				configuration { }
			else
				local str = backtick(sdlconfigcmd() .. " --libs | sed 's/ -lSDLmain//'")
				addlibfromstring(str)
				addoptionsfromstring(str)
			end
			configuration { "x32", "vs*" }
				libdirs {
					path.join(_OPTIONS["SDL_INSTALL_ROOT"],"lib","x86")
				}
			configuration { "x64", "vs*" }
				libdirs {
					path.join(_OPTIONS["SDL_INSTALL_ROOT"],"lib","x64")
				}
			configuration { }
		end
		links {
			"psapi",
		}
	elseif _OPTIONS["targetos"]=="haiku" then
		links {
			"network",
			"bsd",
		}
	end

	configuration { "mingw*" or "vs*" }
		targetprefix "sdl"
		links {
			"psapi",
			"ole32",
		}
	configuration { }

	if _OPTIONS["targetos"]=="macosx" then
		if _OPTIONS["with-bundled-sdl2"]~=nil then
			links {
				"SDL2",
			}
		end
	end

end


function sdlconfigcmd()
	if _OPTIONS["targetos"]=="asmjs" then
		return "sdl2-config"
	elseif not _OPTIONS["SDL_INSTALL_ROOT"] then
		return pkgconfigcmd() .. " sdl2"
	else
		return path.join(_OPTIONS["SDL_INSTALL_ROOT"],"bin","sdl2") .. "-config"
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
	if _OPTIONS["targetos"]=="windows" or _OPTIONS["targetos"]=="macosx" or _OPTIONS["targetos"]=="haiku" or _OPTIONS["targetos"]=="asmjs" then
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
	if _OPTIONS["targetos"]=="windows" or _OPTIONS["targetos"]=="macosx" or _OPTIONS["targetos"]=="haiku" or _OPTIONS["targetos"]=="asmjs" then
		_OPTIONS["NO_USE_XINPUT"] = "1"
	else
		_OPTIONS["NO_USE_XINPUT"] = "0"
	end
end

newoption {
	trigger = "NO_USE_XINPUT_WII_LIGHTGUN_HACK",
	description = "Disable use of Xinput Wii Lightgun Hack",
	allowed = {
		{ "0",  "Enable Xinput Wii Lightgun Hack"  },
		{ "1",  "Disable Xinput Wii Lightgun Hack" },
	},
}

if not _OPTIONS["NO_USE_XINPUT_WII_LIGHTGUN_HACK"] then
	_OPTIONS["NO_USE_XINPUT_WII_LIGHTGUN_HACK"] = "1"
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
	trigger = "USE_LIBSDL",
	description = "Use SDL library on OS (rather than framework/dll)",
	allowed = {
		{ "0",  "Use framework/dll"  },
		{ "1",  "Use library" },
	},
}

if not _OPTIONS["USE_LIBSDL"] then
	_OPTIONS["USE_LIBSDL"] = "0"
end


BASE_TARGETOS       = "unix"
SDLOS_TARGETOS      = "unix"
if _OPTIONS["targetos"]=="linux" then
elseif _OPTIONS["targetos"]=="openbsd" then
elseif _OPTIONS["targetos"]=="netbsd" then
elseif _OPTIONS["targetos"]=="haiku" then
elseif _OPTIONS["targetos"]=="asmjs" then
elseif _OPTIONS["targetos"]=="windows" then
	BASE_TARGETOS       = "win32"
	SDLOS_TARGETOS      = "win32"
elseif _OPTIONS["targetos"]=="macosx" then
	SDLOS_TARGETOS      = "macosx"
end

if _OPTIONS["with-bundled-sdl2"]~=nil then
	includedirs {
		GEN_DIR .. "includes",
	}
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
		if _OPTIONS["with-bundled-sdl2"]~=nil then
			linkoptions {
				"-framework AudioToolbox",
				"-framework AudioUnit",
				"-framework CoreAudio",
				"-framework Carbon",
				"-framework ForceFeedback",
				"-framework IOKit",
				"-framework CoreVideo",
			}
		else
			if _OPTIONS["USE_LIBSDL"]~="1" then
				linkoptions {
					"-F" .. _OPTIONS["SDL_FRAMEWORK_PATH"],
				}
				links {
					"SDL2.framework",
				}
			else
				local str = backtick(sdlconfigcmd() .. " --libs --static | sed 's/-lSDLmain//'")
				addlibfromstring(str)
				addoptionsfromstring(str)
			end
		end
	else
		if _OPTIONS["NO_X11"]=="1" then
			_OPTIONS["USE_QTDEBUG"] = "0"
		else
			libdirs {
				"/usr/X11/lib",
				"/usr/X11R6/lib",
				"/usr/openwin/lib",
			}
		end
		if _OPTIONS["with-bundled-sdl2"]~=nil then
			if _OPTIONS["targetos"]~="android" then
				links {
					"SDL2",
				}
			end
		else
			local str = backtick(sdlconfigcmd() .. " --libs")
			addlibfromstring(str)
			addoptionsfromstring(str)
		end

		if _OPTIONS["targetos"]~="haiku" and _OPTIONS["targetos"]~="android" then
			links {
				"m",
				"pthread",
			}
			if _OPTIONS["targetos"]=="solaris" then
				links {
					"socket",
					"nsl",
				}
			elseif _OPTIONS["targetos"]~="asmjs" then
				links {
					"util",
				}
			end
		end
	end
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
	configuration { "linux-* or freebsd" }
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
		MAME_DIR .. "src/osd/modules/file",
		MAME_DIR .. "src/osd/modules/render",
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "src/osd/sdl",
	}

	if _OPTIONS["targetos"]=="windows" then
		files {
			MAME_DIR .. "src/osd/windows/main.cpp",
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
	end

	files {
		MAME_DIR .. "src/osd/sdl/osdsdl.h",
		MAME_DIR .. "src/osd/sdl/sdlprefix.h",
		MAME_DIR .. "src/osd/sdl/sdlmain.cpp",
		MAME_DIR .. "src/osd/osdepend.h",
		MAME_DIR .. "src/osd/sdl/video.cpp",
		MAME_DIR .. "src/osd/sdl/window.cpp",
		MAME_DIR .. "src/osd/sdl/window.h",
		MAME_DIR .. "src/osd/modules/osdwindow.cpp",
		MAME_DIR .. "src/osd/modules/osdwindow.h",
		MAME_DIR .. "src/osd/modules/render/drawsdl.cpp",
	}
	files {
		MAME_DIR .. "src/osd/modules/render/draw13.cpp",
		MAME_DIR .. "src/osd/modules/render/blit13.h",
	}


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
		MAME_DIR .. "src/osd/osdsync.cpp",
		MAME_DIR .. "src/osd/osdsync.h",
		MAME_DIR .. "src/osd/modules/osdmodule.cpp",
		MAME_DIR .. "src/osd/modules/osdmodule.h",
		MAME_DIR .. "src/osd/modules/lib/osdlib_" .. SDLOS_TARGETOS .. ".cpp",
		MAME_DIR .. "src/osd/modules/lib/osdlib.h",
	}

	if BASE_TARGETOS=="unix" then
		files {
			MAME_DIR .. "src/osd/modules/file/posixdir.cpp",
			MAME_DIR .. "src/osd/modules/file/posixdomain.cpp",
			MAME_DIR .. "src/osd/modules/file/posixfile.cpp",
			MAME_DIR .. "src/osd/modules/file/posixfile.h",
			MAME_DIR .. "src/osd/modules/file/posixptty.cpp",
			MAME_DIR .. "src/osd/modules/file/posixsocket.cpp",
		}
	elseif BASE_TARGETOS=="win32" then
		includedirs {
			MAME_DIR .. "src/osd/windows",
		}
		files {
			MAME_DIR .. "src/osd/modules/file/windir.cpp",
			MAME_DIR .. "src/osd/modules/file/winfile.cpp",
			MAME_DIR .. "src/osd/modules/file/winfile.h",
			MAME_DIR .. "src/osd/modules/file/winptty.cpp",
			MAME_DIR .. "src/osd/modules/file/winsocket.cpp",
			MAME_DIR .. "src/osd/windows/winutil.cpp", -- FIXME put the necessary functions somewhere more appropriate
		}
	else
		files {
			MAME_DIR .. "src/osd/modules/file/stdfile.cpp",
		}
	end


