// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 Magnavox Odyssey cart emulation


 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  o2_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(O2_ROM_STD, o2_rom_device,   "o2_rom",   "Odyssey 2 Standard Carts")
DEFINE_DEVICE_TYPE(O2_ROM_12K, o2_rom12_device, "o2_rom12", "Odyssey 2 12K Carts")
DEFINE_DEVICE_TYPE(O2_ROM_16K, o2_rom16_device, "o2_rom16", "Odyssey 2 16K Carts")


o2_rom_device::o2_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_o2_cart_interface(mconfig, *this)
	, m_bank_base(0)
{
}

o2_rom_device::o2_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: o2_rom_device(mconfig, O2_ROM_STD, tag, owner, clock)
{
}

o2_rom12_device::o2_rom12_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: o2_rom_device(mconfig, O2_ROM_12K, tag, owner, clock)
{
}

o2_rom16_device::o2_rom16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: o2_rom_device(mconfig, O2_ROM_16K, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start/device_reset - device-specific startup
//-------------------------------------------------

void o2_rom_device::device_start()
{
	save_item(NAME(m_bank_base));
}

void o2_rom_device::device_reset()
{
	m_bank_base = 0;
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

void o2_rom_device::write_bank(int bank)
{
	m_bank_base = bank;
}

READ8_MEMBER(o2_rom_device::read_rom04)
{
	return m_rom[(offset + (m_bank_base & 0x03) * 0x800) & (m_rom_size - 1)];
}
READ8_MEMBER(o2_rom_device::read_rom0c)
{
	return m_rom[(offset + (m_bank_base & 0x03) * 0x800) & (m_rom_size - 1)];
}

READ8_MEMBER(o2_rom12_device::read_rom04)
{
	return m_rom[offset + (m_bank_base & 0x03) * 0xc00];
}
READ8_MEMBER(o2_rom12_device::read_rom0c)
{
	return m_rom[offset + 0x800 + (m_bank_base & 0x03) * 0xc00];
}

READ8_MEMBER(o2_rom16_device::read_rom04)
{
	return m_rom[offset + 0x400 + (m_bank_base & 0x03) * 0x1000];
}
READ8_MEMBER(o2_rom16_device::read_rom0c)
{
	return m_rom[offset + 0xc00 + (m_bank_base & 0x03) * 0x1000];
}
