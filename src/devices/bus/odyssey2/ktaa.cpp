// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Fabio Priuli, hap
/******************************************************************************

Homebrew KTAA(Kill the Attacking Aliens) cartridge emulation

Bankswitched ROM with page size of 3KB.

******************************************************************************/

#include "emu.h"
#include "ktaa.h"

DEFINE_DEVICE_TYPE(O2_ROM_KTAA, o2_ktaa_device, "o2_ktaa", "Videopac+ KTAA Cartridge")


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
	bool err = false;

	if (m_rom_size & (m_rom_size - 1))
	{
		// freely released binary file is 12KB
		err = m_rom_size != 0xc00 && m_rom_size != 0xc00*2 && m_rom_size != 0xc00*4;
		m_page_size = 0xc00;
	}
	else
	{
		// actual ROM is 16KB(27C128), first 1KB of each 4KB block is empty
		err = m_rom_size < 0x1000;
		m_page_size = 0x1000;
	}

	if (err)
		fatalerror("o2_ktaa_device: ROM size must be multiple of 3KB or 4KB\n");

	m_bank_mask = (m_rom_size / m_page_size) - 1;
}


//-------------------------------------------------
//  mapper specific handlers
//-------------------------------------------------

u8 o2_ktaa_device::read_rom04(offs_t offset)
{
	offset += m_page_size - 0xc00;
	return m_rom[offset + m_bank * m_page_size];
}
