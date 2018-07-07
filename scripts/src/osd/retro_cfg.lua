-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

dofile("retro_modules.lua")

forcedincludes {
	MAME_DIR .. "src/osd/libretro/retroprefix.h"
}

if SDL_NETWORK~="" and not _OPTIONS["DONT_USE_NETWORK"] then
	defines {
		"USE_NETWORK",
		"OSD_NET_USE_" .. string.upper(SDL_NETWORK),
	}
end

if _OPTIONS["RETRO_INI_PATH"]~=nil then
	defines {
		"'INI_PATH=\"" .. _OPTIONS["RETRO_INI_PATH"] .. "\"'",
	}
end

	defines {
		"SDLMAME_NO_X11",
		"RETROMAME",
	}


	defines {
		"USE_XINPUT=0",
	}


defines {
	"OSD_RETRO",
}

if BASE_TARGETOS=="unix" then
	defines {
		"SDLMAME_UNIX",
		"RETROMAME_UNIX",
	}

end

if _OPTIONS["targetos"]=="windows" then
	configuration { "mingw* or vs*" }
		defines {
			"UNICODE",
			"_UNICODE",
			"main=utf8_main",
		        "WIN32_LEAN_AND_MEAN",
		}
	
	configuration { "mingw*" }
		defines {
			"_WIN32_WINNT=0x0501",
		}

	configuration { "Debug" }
		defines {
			"MALLOC_DEBUG",
		}
	configuration { }


elseif _OPTIONS["targetos"]=="macosx" then
	defines {
		"SDLMAME_MACOSX",
		"SDLMAME_DARWIN",
	}
elseif _OPTIONS["targetos"]=="freebsd" then
	buildoptions {
		-- /usr/local/include is not considered a system include director on FreeBSD.  GL.h resides there and throws warnings
		"-isystem /usr/local/include",
	}
end

configuration { "osx*" }
	includedirs {
		MAME_DIR .. "3rdparty/bx/include/compat/osx",
	}

configuration { "freebsd" }
	includedirs {
		MAME_DIR .. "3rdparty/bx/include/compat/freebsd",
	}

configuration { "netbsd" }
	includedirs {
		MAME_DIR .. "3rdparty/bx/include/compat/freebsd",
	}

configuration { }

