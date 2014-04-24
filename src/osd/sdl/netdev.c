#include "emu.h"
#include "netdev_tap.h"
#include "netdev_pcap.h"
#include "osdsdl.h"

bool sdl_osd_interface::network_init()
{
	#ifdef SDLMAME_NET_TAPTUN
	init_tap();
	#endif
	#ifdef SDLMAME_NET_PCAP
	init_pcap();
	#endif
	return true;
}

void sdl_osd_interface::network_exit()
{
	#ifdef SDLMAME_NET_TAPTUN
	deinit_tap();
	#endif
	#ifdef SDLMAME_NET_PCAP
	deinit_pcap();
	#endif
}
