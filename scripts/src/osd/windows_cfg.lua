defines {
	"OSD_WINDOWS",
	"USE_SDL=0",
	"_WIN32_WINNT=0x0501",
}

if not _OPTIONS["DONT_USE_NETWORK"] then
	defines {
		"USE_NETWORK",
		"OSD_NET_USE_PCAP",
	}
end
