// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "segabb.h"

DEFINE_DEVICE_TYPE(SEGA_LINDBERGH_BASEBOARD, sega_lindbergh_baseboard_device, "lindbergh_baseboard", "Sega Lindbergh Baseboard")

void sega_lindbergh_baseboard_device::map1(address_map &map)
{
}

void sega_lindbergh_baseboard_device::map2(address_map &map)
{
}

void sega_lindbergh_baseboard_device::map3(address_map &map)
{
}

sega_lindbergh_baseboard_device::sega_lindbergh_baseboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, SEGA_LINDBERGH_BASEBOARD, tag, owner, clock)
{
	set_ids(0x105718c1, 0x10, 0x068000, 0x11db067b);
}

void sega_lindbergh_baseboard_device::device_start()
{
	pci_device::device_start();
	add_map(   128*1024, M_MEM, FUNC(sega_lindbergh_baseboard_device::map1));
	add_map(  1024*1024, M_MEM, FUNC(sega_lindbergh_baseboard_device::map2));
	add_map(2*1024*1024, M_MEM, FUNC(sega_lindbergh_baseboard_device::map3));
}

void sega_lindbergh_baseboard_device::device_reset()
{
	pci_device::device_reset();
}
