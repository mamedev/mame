-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

dofile('modules.lua')

forcedincludes {
--  MAME_DIR .. "src/osd/sdl/sdlprefix.h"
}

if _OPTIONS["USE_TAPTUN"]=="1" or _OPTIONS["USE_PCAP"]=="1" then
	defines {
		"USE_NETWORK",
	}
	if _OPTIONS["USE_TAPTUN"]=="1" then
		defines {
			"OSD_NET_USE_TAPTUN",
		}
	end
	if _OPTIONS["USE_PCAP"]=="1" then
		defines {
			"OSD_NET_USE_PCAP",
		}
	end
end

defines {
	"OSD_MAC",
	"SDLMAME_UNIX",
	"SDLMAME_MACOSX",
	"SDLMAME_DARWIN"
}

configuration { "osx*" }
	includedirs {
		MAME_DIR .. "3rdparty/bx/include/compat/osx",
	}


configuration { }

