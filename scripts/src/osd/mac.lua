-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   mac.lua
--
--   Rules for the building with SDL
--
---------------------------------------------------------------------------

dofile("modules.lua")


function maintargetosdoptions(_target,_subtarget)
	osdmodulestargetconf()

	configuration { }
end

BASE_TARGETOS       = "unix"

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

project ("qtdbg_" .. _OPTIONS["osd"])
	uuid (os.uuid("qtdbg_" .. _OPTIONS["osd"]))
	kind (LIBTYPE)

	dofile("mac_cfg.lua")
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

	dofile("mac_cfg.lua")
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
		MAME_DIR .. "src/osd/mac",
	}

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

	files {
		MAME_DIR .. "src/osd/mac/main.mm",
		MAME_DIR .. "src/osd/mac/macmain.cpp",
		MAME_DIR .. "src/osd/mac/appdelegate.mm",
		MAME_DIR .. "src/osd/mac/appdelegate.h",
		MAME_DIR .. "src/osd/mac/video.cpp",
		MAME_DIR .. "src/osd/mac/window.cpp",
		MAME_DIR .. "src/osd/mac/window.h",
		MAME_DIR .. "src/osd/mac/windowcontroller.mm",
		MAME_DIR .. "src/osd/mac/windowcontroller.h",
		MAME_DIR .. "src/osd/mac/mamefswindow.mm",
		MAME_DIR .. "src/osd/mac/mamefswindow.h",
		MAME_DIR .. "src/osd/mac/oglview.mm",
		MAME_DIR .. "src/osd/mac/oglview.h",
		MAME_DIR .. "src/osd/modules/osdwindow.cpp",
		MAME_DIR .. "src/osd/modules/osdwindow.h",
	}


project ("ocore_" .. _OPTIONS["osd"])
	targetsubdir(_OPTIONS["target"] .."_" .. _OPTIONS["subtarget"])
	uuid (os.uuid("ocore_" .. _OPTIONS["osd"]))
	kind (LIBTYPE)

	removeflags {
		"SingleOutputDir",
	}

	dofile("mac_cfg.lua")

	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "src/osd/mac",
	}

	files {
		MAME_DIR .. "src/osd/osdcore.cpp",
		MAME_DIR .. "src/osd/osdcore.h",
		MAME_DIR .. "src/osd/osdfile.h",
		MAME_DIR .. "src/osd/strconv.cpp",
		MAME_DIR .. "src/osd/strconv.h",
		MAME_DIR .. "src/osd/osdsync.cpp",
		MAME_DIR .. "src/osd/osdsync.h",
		MAME_DIR .. "src/osd/modules/osdmodule.cpp",
		MAME_DIR .. "src/osd/modules/osdmodule.h",
		MAME_DIR .. "src/osd/modules/lib/osdlib_macosx.cpp",
		MAME_DIR .. "src/osd/modules/lib/osdlib.h",
		MAME_DIR .. "src/osd/modules/file/posixdir.cpp",
		MAME_DIR .. "src/osd/modules/file/posixdomain.cpp",
		MAME_DIR .. "src/osd/modules/file/posixfile.cpp",
		MAME_DIR .. "src/osd/modules/file/posixfile.h",
		MAME_DIR .. "src/osd/modules/file/posixptty.cpp",
		MAME_DIR .. "src/osd/modules/file/posixsocket.cpp",
	}


