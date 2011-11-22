#include "emu.h"
#include "netdev_tap.h"
#include "netdev_pcap.h"

void sdlnetdev_init(running_machine &machine)
{
    #ifdef SDLMAME_NET_TAPTUN
	init_tap();
    #endif
    #ifdef SDLMAME_NET_PCAP
    init_pcap();
    #endif
}

void sdlnetdev_deinit(running_machine &machine)
{
    #ifdef SDLMAME_NET_TAPTUN
	deinit_tap();
    #endif
    #ifdef SDLMAME_NET_PCAP
    deinit_pcap();
    #endif
}
