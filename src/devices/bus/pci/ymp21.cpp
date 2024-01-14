// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "ymp21.h"

ymp21_device::ymp21_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
{
}

void ymp21_device::device_start()
{
	pci_card_device::device_start();
	add_map(0x40000, M_MEM, FUNC(ymp21_device::map));
	intr_pin = 0x01;
}

void ymp21_device::device_reset()
{
	pci_card_device::device_reset();
}

void ymp21_device::map(address_map &map)
{
	map(0x00000, 0x3ffff).rw(FUNC(ymp21_device::read), FUNC(ymp21_device::write));
}

u32 ymp21_device::read(offs_t offset, u32 mem_mask)
{
	logerror("ym read %05x %08x\n", offset*4, mem_mask);
	return 0;
}

void ymp21_device::write(offs_t offset, u32 data, u32 mem_mask)
{
	logerror("ym write %05x, %08x %08x\n", offset*4, data, mem_mask);
}


