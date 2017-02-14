// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "mga2064w.h"

const device_type MGA2064W = &device_creator<mga2064w_device>;

mga2064w_device::mga2064w_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MGA2064W, "Matrox Millennium", tag, owner, clock, "mga2064w", __FILE__)
{
}

void mga2064w_device::device_start()
{
	pci_device::device_start();
	//  add_map( 16*1024*1024, M_MEM, FUNC(mga2064w_device::map1));
	//  add_map(256*1024*1024, M_MEM, FUNC(mga2064w_device::map2));
	//  add_map( 16*1024*1024, M_MEM, FUNC(mga2064w_device::map3));
	//  add_rom_from_region();
}

void mga2064w_device::device_reset()
{
	pci_device::device_reset();
}
