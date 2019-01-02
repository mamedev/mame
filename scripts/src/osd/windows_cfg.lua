-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

defines {
	"OSD_WINDOWS",
	"WIN32_LEAN_AND_MEAN",
	"NOMINMAX",
}

configuration { "mingw* or vs*" }
	defines {
		"UNICODE",
		"_UNICODE"
	}

configuration { "vs*" }
	flags {
		"Unicode",
	}

configuration { }

if not _OPTIONS["MODERN_WIN_API"] then
	_OPTIONS["MODERN_WIN_API"] = "0"
end

if _OPTIONS["MODERN_WIN_API"]=="1" then
	defines {
		"WINVER=0x0602",
		"_WIN32_WINNT=0x0602",
		"NTDDI_VERSION=0x06030000",
		"MODERN_WIN_API",
	}
else
	defines {
		"_WIN32_WINNT=0x0501",
	}
end

if not _OPTIONS["DONT_USE_NETWORK"] then
	defines {
		"USE_NETWORK",
		"OSD_NET_USE_PCAP",
		"OSD_NET_USE_TAPTUN",
	}
end

if _OPTIONS["USE_SDL"]=="1" then
	defines {
		"SDLMAME_SDL2=1",
		"USE_XINPUT=0",
		"USE_SDL=1",
		"USE_SDL_SOUND",
	}
else
	defines {
		"USE_SDL=0",
	}
end
