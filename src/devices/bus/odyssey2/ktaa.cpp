// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Fabio Priuli, hap
/******************************************************************************

Homebrew KTAA(Kill the Attacking Aliens) cartridge emulation

Bankswitched ROM with page size of 3KB.

******************************************************************************/

#include "emu.h"
#include "ktaa.h"

DEFINE_DEVICE_TYPE(O2_ROM_KTAA, o2_ktaa_device, "o2_ktaa", "Odyssey 2 Homebrew KTAA")


//-------------------------------------------------
//  o2_ktaa_device - constructor
//-------------------------------------------------

o2_ktaa_device::o2_ktaa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, O2_ROM_KTAA, tag, owner, clock),
	device_o2_cart_interface(mconfig, *this)
{ }

void o2_ktaa_device::device_start()
{
	save_item(NAME(m_bank));
}

void o2_ktaa_device::cart_init()
{
	u32 size = m_rom_size;
	if (size != 0xc00 && size != 0xc00*2 && size != 0xc00*4)
		fatalerror("o2_ktaa_device: ROM size must be multiple of 3KB\n");

	m_bank_mask = (size / 0xc00) - 1;
}


//-------------------------------------------------
//  mapper specific handlers
//-------------------------------------------------

u8 o2_ktaa_device::read_rom04(offs_t offset)
{
	return m_rom[offset + m_bank * 0xc00];
}

u8 o2_ktaa_device::read_rom0c(offs_t offset)
{
	return m_rom[offset + 0x800 + m_bank * 0xc00];
}
