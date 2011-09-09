#include "emu.h"
#include "netdev_pcap.h"

void winnetdev_init(running_machine &machine)
{
	init_pcap();
}
