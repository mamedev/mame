-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   sdl.lua
--
--   Rules for the building with SDL
--
---------------------------------------------------------------------------

dofile("retro_modules.lua")

function maintargetosdoptions(_target,_subtarget)
	osdmodulestargetconf()
end

forcedincludes {
	MAME_DIR .. "src/osd/libretro/retroprefix.h"
}

newoption {
	trigger = "RETRO_INI_PATH",
	description = "Default search path for .ini files",
}

BASE_TARGETOS       = "unix"
SDLOS_TARGETOS      = "unix"
SDL_NETWORK         = ""
if _OPTIONS["targetos"]=="linux" then
	SDL_NETWORK         = "taptun"
elseif _OPTIONS["targetos"]=="openbsd" then
elseif _OPTIONS["targetos"]=="netbsd" then
	SDL_NETWORK         = "pcap"
elseif _OPTIONS["targetos"]=="haiku" then
elseif _OPTIONS["targetos"]=="asmjs" then
elseif _OPTIONS["targetos"]=="windows" then
	BASE_TARGETOS       = "win32"
	SDLOS_TARGETOS      = "win32"
--	SDL_NETWORK         = "pcap"
elseif _OPTIONS["targetos"]=="macosx" then
	SDLOS_TARGETOS      = "macosx"
	SDL_NETWORK         = "pcap"
end

if BASE_TARGETOS=="unix" then

			_OPTIONS["USE_QTDEBUG"] = "0"



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
			else
				links {
					"util",
				}
			end
		end
	
end

project ("osd_" .. _OPTIONS["osd"])
	targetsubdir(_OPTIONS["target"] .."_" .._OPTIONS["subtarget"])
	uuid (os.uuid("osd_" .. _OPTIONS["osd"]))
--	kind (LIBTYPE)
	kind "StaticLib"

	if string.sub(_ACTION,1,4) ~= "vs20" then
		buildoptions {
			"-fPIC"
		}

		linkoptions{
			"-shared -Wl,--version-script=" .. MAME_DIR .. "src/osd/libretro/libretro-internal/link.T -Wl,--no-undefined"
		}
	end

	dofile("retro_cfg.lua")
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
		MAME_DIR .. "3rdparty/bgfx/include",
		MAME_DIR .. "src/osd/libretro",
		MAME_DIR .. "src/osd/libretro/libretro-internal",
	}

	if _OPTIONS["targetos"]=="windows" then
		files {
	--		MAME_DIR .. "src/osd/windows/main.cpp",
		}
	end

	files {
		MAME_DIR .. "src/osd/libretro/libretro-internal/retro_init.cpp",
		MAME_DIR .. "src/osd/libretro/osdretro.h",
		MAME_DIR .. "src/osd/libretro/retroprefix.h",
		MAME_DIR .. "src/osd/libretro/retromain.cpp",
		MAME_DIR .. "src/osd/osdepend.h",
		MAME_DIR .. "src/osd/libretro/video.cpp",
		MAME_DIR .. "src/osd/libretro/window.cpp",
		MAME_DIR .. "src/osd/libretro/window.h",
		MAME_DIR .. "src/osd/modules/osdwindow.cpp",
		MAME_DIR .. "src/osd/modules/osdwindow.h",
		MAME_DIR .. "src/osd/modules/render/drawretro.cpp",
		MAME_DIR .. "src/osd/modules/sound/retro_sound.cpp",
	}


project ("ocore_" .. _OPTIONS["osd"])
	targetsubdir(_OPTIONS["target"] .."_" .. _OPTIONS["subtarget"])
	uuid (os.uuid("ocore_" .. _OPTIONS["osd"]))
--	kind (LIBTYPE)
	kind "StaticLib"

	removeflags {
		"SingleOutputDir",
	}

	dofile("retro_cfg.lua")

	if string.sub(_ACTION,1,4) ~= "vs20" then
		buildoptions {
			"-fPIC"
		}
   
		linkoptions{
			"-shared -Wl,--version-script=src/osd/libretro/libretro-internal/link.T -Wl,--no-undefined"
		}
	end
	
	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "src/osd/libretro",
		MAME_DIR .. "src/osd/libretro/libretro-internal",
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
		MAME_DIR .. "src/osd/modules/lib/osdlib_retro.cpp",
		MAME_DIR .. "src/osd/osdcore.cpp",
		MAME_DIR .. "src/osd/osdcore.h",
		MAME_DIR .. "src/osd/strconv.cpp",
		MAME_DIR .. "src/osd/strconv.h",
		MAME_DIR .. "src/osd/osdsync.cpp",
		MAME_DIR .. "src/osd/osdsync.h",
		MAME_DIR .. "src/osd/modules/osdmodule.cpp",
		MAME_DIR .. "src/osd/modules/osdmodule.h",
		MAME_DIR .. "src/osd/modules/lib/osdlib.h",
	}

	if BASE_TARGETOS=="unix" then
		files {
			MAME_DIR .. "src/osd/modules/file/posixdir.cpp",
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


