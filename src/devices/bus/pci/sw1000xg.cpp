// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "sw1000xg.h"

DEFINE_DEVICE_TYPE(SW1000XG, sw1000xg_device, "sw1000xg", "Yamaha SW1000XG")

sw1000xg_device::sw1000xg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, SW1000XG, tag, owner, clock)
{
	set_ids(0x10731000, 0x00, 0x040100, 0x10731000);
}

void sw1000xg_device::device_start()
{
	pci_device::device_start();
	add_map(0x40000, M_MEM, FUNC(sw1000xg_device::map));
	intr_pin = 0x01;
}

void sw1000xg_device::device_reset()
{
}

void sw1000xg_device::map(address_map &map)
{
	map(0x00000, 0x3ffff).rw(FUNC(sw1000xg_device::read), FUNC(sw1000xg_device::write));
}

u32 sw1000xg_device::read(offs_t offset, u32 mem_mask)
{
	logerror("ym read %05x %08x\n", offset*4, mem_mask);
	return 0;
}

void sw1000xg_device::write(offs_t offset, u32 data, u32 mem_mask)
{
	logerror("ym write %05x, %08x %08x\n", offset*4, data, mem_mask);
}


