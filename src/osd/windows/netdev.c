#include "emu.h"
#include "winmain.h"
#include "netdev_pcap.h"

bool windows_osd_interface::network_init()
{
	init_pcap();
	return true;
}

void windows_osd_interface::network_exit()
{
	deinit_pcap();
}
