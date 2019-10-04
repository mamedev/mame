-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

dofile('modules.lua')

forcedincludes {
--  MAME_DIR .. "src/osd/sdl/sdlprefix.h"
}

if not _OPTIONS["DONT_USE_NETWORK"] then
	defines {
		"USE_NETWORK",
		"OSD_NET_USE_PCAP",
	}
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

