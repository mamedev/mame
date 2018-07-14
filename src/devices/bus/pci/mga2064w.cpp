// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "mga2064w.h"

DEFINE_DEVICE_TYPE(MGA2064W, mga2064w_device, "mga2064w", "Matrox Millennium")

mga2064w_device::mga2064w_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MGA2064W, tag, owner, clock)
{
	set_ids(0x102b0519, 0x01, 0x030000, 0x00000000);
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
