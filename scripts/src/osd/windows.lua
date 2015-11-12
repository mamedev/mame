-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

dofile("modules.lua")


function maintargetosdoptions(_target,_subtarget)
	osdmodulestargetconf()

	configuration { "mingw*-gcc" }
		linkoptions {
			"-municode",
		}
		links {
			"mingw32",
		}

	configuration { }

	if _OPTIONS["DIRECTINPUT"] == "8" then
		links {
			"dinput8",
		}
	else
		links {
			"dinput",
		}
	end


	if _OPTIONS["USE_SDL"] == "1" then
		links {
			"SDL.dll",
		}
	end

	links {
		"comctl32",
		"comdlg32",
		"psapi",
	}
end


newoption {
	trigger = "DIRECTINPUT",
	description = "Minimum DirectInput version to support",
	allowed = {
		{ "7",  "Support DirectInput 7 or later"  },
		{ "8",  "Support DirectInput 8 or later"  },
	},
}

if not _OPTIONS["DIRECTINPUT"] then
	_OPTIONS["DIRECTINPUT"] = "8"
end

newoption {
	trigger = "USE_SDL",
	description = "Enable SDL sound output",
	allowed = {
		{ "0",  "Disable SDL sound output"  },
		{ "1",  "Enable SDL sound output"   },
	},
}

if not _OPTIONS["USE_SDL"] then
	_OPTIONS["USE_SDL"] = "0"
end

newoption {
	trigger = "CYGWIN_BUILD",
	description = "Build with Cygwin tools",
	allowed = {
		{ "0",  "Build with MinGW tools"   },
		{ "1",  "Build with Cygwin tools"  },
	},
}

if not _OPTIONS["CYGWIN_BUILD"] then
	_OPTIONS["CYGWIN_BUILD"] = "0"
end


if _OPTIONS["CYGWIN_BUILD"] == "1" then
	buildoptions {
		"-mmo-cygwin",
	}
	linkoptions {
		"-mno-cygwin",
	}
end


project ("osd_" .. _OPTIONS["osd"])
	uuid (os.uuid("osd_" .. _OPTIONS["osd"]))
	kind (LIBTYPE)

	dofile("windows_cfg.lua")
	osdmodulesbuild()

	defines {
		"DIRECT3D_VERSION=0x0900",
	}

	if _OPTIONS["DIRECTINPUT"] == "8" then
		defines {
			"DIRECTINPUT_VERSION=0x0800",
		}
	else
		defines {
			"DIRECTINPUT_VERSION=0x0700",
		}
	end

	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices", -- accessing imagedev from debugger
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "src/osd/modules/render",
		MAME_DIR .. "3rdparty",
	}

	includedirs {
		MAME_DIR .. "src/osd/windows",
	}

	files {
		MAME_DIR .. "src/osd/modules/render/drawd3d.cpp",
		MAME_DIR .. "src/osd/modules/render/drawd3d.h",
		MAME_DIR .. "src/osd/modules/render/d3d/d3d9intf.cpp",
		MAME_DIR .. "src/osd/modules/render/d3d/d3dhlsl.cpp",
		MAME_DIR .. "src/osd/modules/render/d3d/d3dcomm.h",
		MAME_DIR .. "src/osd/modules/render/d3d/d3dhlsl.h",
		MAME_DIR .. "src/osd/modules/render/d3d/d3dintf.h",
		MAME_DIR .. "src/osd/modules/render/drawdd.cpp",
		MAME_DIR .. "src/osd/modules/render/drawgdi.cpp",
		MAME_DIR .. "src/osd/modules/render/drawnone.cpp",
		MAME_DIR .. "src/osd/windows/input.cpp",
		MAME_DIR .. "src/osd/windows/input.h",
		MAME_DIR .. "src/osd/windows/output.cpp",
		MAME_DIR .. "src/osd/windows/output.h",
		MAME_DIR .. "src/osd/windows/video.cpp",
		MAME_DIR .. "src/osd/windows/video.h",
		MAME_DIR .. "src/osd/windows/window.cpp",
		MAME_DIR .. "src/osd/windows/window.h",
		MAME_DIR .. "src/osd/modules/osdwindow.h",
		MAME_DIR .. "src/osd/windows/winmenu.cpp",
		MAME_DIR .. "src/osd/windows/winmain.cpp",
		MAME_DIR .. "src/osd/windows/winmain.h",
		MAME_DIR .. "src/osd/osdepend.h",
		MAME_DIR .. "src/osd/modules/debugger/win/consolewininfo.cpp",
		MAME_DIR .. "src/osd/modules/debugger/win/consolewininfo.h",
		MAME_DIR .. "src/osd/modules/debugger/win/debugbaseinfo.cpp",
		MAME_DIR .. "src/osd/modules/debugger/win/debugbaseinfo.h",
		MAME_DIR .. "src/osd/modules/debugger/win/debugviewinfo.cpp",
		MAME_DIR .. "src/osd/modules/debugger/win/debugviewinfo.h",
		MAME_DIR .. "src/osd/modules/debugger/win/debugwininfo.cpp",
		MAME_DIR .. "src/osd/modules/debugger/win/debugwininfo.h",
		MAME_DIR .. "src/osd/modules/debugger/win/disasmbasewininfo.cpp",
		MAME_DIR .. "src/osd/modules/debugger/win/disasmbasewininfo.h",
		MAME_DIR .. "src/osd/modules/debugger/win/disasmviewinfo.cpp",
		MAME_DIR .. "src/osd/modules/debugger/win/disasmviewinfo.h",
		MAME_DIR .. "src/osd/modules/debugger/win/disasmwininfo.cpp",
		MAME_DIR .. "src/osd/modules/debugger/win/disasmwininfo.h",
		MAME_DIR .. "src/osd/modules/debugger/win/editwininfo.cpp",
		MAME_DIR .. "src/osd/modules/debugger/win/editwininfo.h",
		MAME_DIR .. "src/osd/modules/debugger/win/logwininfo.cpp",
		MAME_DIR .. "src/osd/modules/debugger/win/logwininfo.h",
		MAME_DIR .. "src/osd/modules/debugger/win/memoryviewinfo.cpp",
		MAME_DIR .. "src/osd/modules/debugger/win/memoryviewinfo.h",
		MAME_DIR .. "src/osd/modules/debugger/win/memorywininfo.cpp",
		MAME_DIR .. "src/osd/modules/debugger/win/memorywininfo.h",
		MAME_DIR .. "src/osd/modules/debugger/win/pointswininfo.cpp",
		MAME_DIR .. "src/osd/modules/debugger/win/pointswininfo.h",
		MAME_DIR .. "src/osd/modules/debugger/win/uimetrics.cpp",
		MAME_DIR .. "src/osd/modules/debugger/win/uimetrics.h",
		MAME_DIR .. "src/osd/modules/debugger/win/debugwin.h",
	}


project ("ocore_" .. _OPTIONS["osd"])
	uuid (os.uuid("ocore_" .. _OPTIONS["osd"]))
	kind (LIBTYPE)

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
		MAME_DIR .. "src/osd/eigccppc.h",
		MAME_DIR .. "src/osd/eigccx86.h",
		MAME_DIR .. "src/osd/eivc.h",
		MAME_DIR .. "src/osd/eivcx86.h",
		MAME_DIR .. "src/osd/eminline.h",
		MAME_DIR .. "src/osd/osdcomm.h",
		MAME_DIR .. "src/osd/osdcore.cpp",
		MAME_DIR .. "src/osd/osdcore.h",
		MAME_DIR .. "src/osd/strconv.cpp",
		MAME_DIR .. "src/osd/strconv.h",
		MAME_DIR .. "src/osd/windows/main.cpp",
		MAME_DIR .. "src/osd/windows/windir.cpp",
		MAME_DIR .. "src/osd/windows/winfile.cpp",
		MAME_DIR .. "src/osd/modules/sync/sync_windows.cpp",
		MAME_DIR .. "src/osd/modules/sync/osdsync.h",
		MAME_DIR .. "src/osd/windows/winutf8.cpp",
		MAME_DIR .. "src/osd/windows/winutf8.h",
		MAME_DIR .. "src/osd/windows/winutil.cpp",
		MAME_DIR .. "src/osd/windows/winutil.h",
		MAME_DIR .. "src/osd/windows/winfile.h",
		MAME_DIR .. "src/osd/windows/winclip.cpp",
		MAME_DIR .. "src/osd/windows/winsocket.cpp",
		MAME_DIR .. "src/osd/windows/winptty.cpp",
		MAME_DIR .. "src/osd/modules/osdmodule.cpp",
		MAME_DIR .. "src/osd/modules/osdmodule.h",
		MAME_DIR .. "src/osd/modules/lib/osdlib_win32.cpp",
	}

	if _OPTIONS["NOASM"] == "1" then
		files {
			MAME_DIR .. "src/osd/modules/sync/work_mini.cpp",
		}
	else
		files {
			MAME_DIR .. "src/osd/modules/sync/work_osd.cpp",
		}
	end


--------------------------------------------------
-- ledutil
--------------------------------------------------

if _OPTIONS["with-tools"] then
	project("ledutil")
		uuid ("061293ca-7290-44ac-b2b5-5913ae8dc9c0")
		kind "ConsoleApp"

		flags {
			"Symbols", -- always include minimum symbols for executables 	
		}

		if _OPTIONS["SEPARATE_BIN"]~="1" then 
			targetdir(MAME_DIR)
		end

		links {
			"ocore_" .. _OPTIONS["osd"],
		}

		includedirs {
			MAME_DIR .. "src/osd",
		}
		
		files {
			MAME_DIR .. "src/osd/windows/ledutil.cpp",
		}
end
