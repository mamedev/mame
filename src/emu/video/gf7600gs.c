// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "gf7600gs.h"

const device_type GEFORCE_7600GS = &device_creator<geforce_7600gs_device>;

DEVICE_ADDRESS_MAP_START(map1, 32, geforce_7600gs_device)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(map2, 32, geforce_7600gs_device)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(map3, 32, geforce_7600gs_device)
ADDRESS_MAP_END

geforce_7600gs_device::geforce_7600gs_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, GEFORCE_7600GS, "NVidia GeForce 7600GS", tag, owner, clock, "geforce_7600gs", __FILE__)
{
}

void geforce_7600gs_device::device_start()
{
	pci_device::device_start();
	add_map( 16*1024*1024, M_MEM, FUNC(geforce_7600gs_device::map1));
	add_map(256*1024*1024, M_MEM, FUNC(geforce_7600gs_device::map2));
	add_map( 16*1024*1024, M_MEM, FUNC(geforce_7600gs_device::map3));
	add_rom_from_region();
}

void geforce_7600gs_device::device_reset()
{
	pci_device::device_reset();
}
