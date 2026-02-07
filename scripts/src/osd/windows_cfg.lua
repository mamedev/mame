-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

defines {
	"OSD_WINDOWS",
	"UNICODE",
	"_UNICODE",
	"WIN32_LEAN_AND_MEAN",
	"NOMINMAX",
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
		"_WIN32_WINNT=0x0602",
		"NTDDI_VERSION=0x06000000",
	}
end

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

if _OPTIONS["USE_SDL"]=="1" then
	defines {
		"SDLMAME_SDL2=1",
		"USE_XINPUT=0",
		"USE_SDL=1",
		"USE_SDL_SOUND",
	}
else
	if _OPTIONS["USE_SDL3"]=="1" then
	defines {
		"SDLMAME_SDL3=1",
		"USE_XINPUT=0",
		"USE_SDL3=1",
		"USE_SDL_SOUND",
	}
	else
		defines {
			"USE_SDL=0",
		}
	end
end
