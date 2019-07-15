// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "gf7600gs.h"

DEFINE_DEVICE_TYPE(GEFORCE_7600GS, geforce_7600gs_device, "geforce_7600gs", "NVIDIA GeForce 7600GS")

void geforce_7600gs_device::map1(address_map &map)
{
}

void geforce_7600gs_device::map2(address_map &map)
{
}

void geforce_7600gs_device::map3(address_map &map)
{
}

geforce_7600gs_device::geforce_7600gs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, GEFORCE_7600GS, tag, owner, clock)
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
