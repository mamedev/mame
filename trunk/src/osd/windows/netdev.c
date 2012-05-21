#include "emu.h"
#include "netdev_pcap.h"

void winnetdev_init(running_machine &machine)
{
	init_pcap();
}

void winnetdev_deinit(running_machine &machine)
{
	deinit_pcap();
}
