// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "segabb.h"

const device_type SEGA_LINDBERGH_BASEBOARD = &device_creator<sega_lindbergh_baseboard_device>;

DEVICE_ADDRESS_MAP_START(map1, 32, sega_lindbergh_baseboard_device)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(map2, 32, sega_lindbergh_baseboard_device)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(map3, 32, sega_lindbergh_baseboard_device)
ADDRESS_MAP_END

sega_lindbergh_baseboard_device::sega_lindbergh_baseboard_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, SEGA_LINDBERGH_BASEBOARD, "Sega Lindbergh Baseboard", tag, owner, clock, "sega_lindbergh_baseboard", __FILE__)
{
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
