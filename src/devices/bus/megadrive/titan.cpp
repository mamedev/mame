// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Titan Overdrive 2 mapper

https://plutiedev.com/beyond-4mb

TODO:
- This is same as SSF2 mapper, merge at cart slot rewrite;

**************************************************************************************************/


#include "emu.h"
#include "titan.h"

DEFINE_DEVICE_TYPE(MD_ROM_TITAN, md_rom_titan_device, "md_rom_titan", "MD Titan Overdrive 2")

md_rom_titan_device::md_rom_titan_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MD_ROM_TITAN, tag, owner, clock)
	, device_md_cart_interface(mconfig, *this)
{
}

void md_rom_titan_device::device_start()
{
	for (int i = 0; i < 8; i++)
		m_bank[i] = i;

	save_pointer(NAME(m_bank), 8);
}

uint16_t md_rom_titan_device::read(offs_t offset)
{
	if (offset < 0x400000/2)
	{
		const u32 bank_offset = offset >> (2 + 16);
		const u32 bank_mask = 0x3ffff;
		const u32 bank_mult = 0x40000;
		//if (bank_offset != 0)
		//  printf("%08x %d %08x\n", offset * 2, bank_offset, m_rom_size);
		return m_rom[((offset & bank_mask) | (m_bank[bank_offset] * bank_mult)) & (m_rom_size - 1)];
	}

	return 0xffff;
}

void md_rom_titan_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
}

void md_rom_titan_device::write_a13(offs_t offset, uint16_t data)
{
	if (offset >= 0xf0/2)
	{
		offset -= 0xf0/2;
		// bank 0 is fixed at 0-0x7ffff
		if (offset)
		{
			//printf("%02x %02x\n", offset, data);
			// D7-D6 unconnected, sgdk_bap (and likely others) cares about D4
			m_bank[offset] = data & 0x3f;
		}
	}
}

