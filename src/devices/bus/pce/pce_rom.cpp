// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 PC-Engine & Turbografx-16 cart emulation


 ***********************************************************************************************************/


#include "emu.h"
#include "pce_rom.h"


//-------------------------------------------------
//  pce_rom_device - constructor
//-------------------------------------------------

const device_type PCE_ROM_STD = &device_creator<pce_rom_device>;
const device_type PCE_ROM_CDSYS3 = &device_creator<pce_cdsys3_device>;
const device_type PCE_ROM_POPULOUS = &device_creator<pce_populous_device>;
const device_type PCE_ROM_SF2 = &device_creator<pce_sf2_device>;


pce_rom_device::pce_rom_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_pce_cart_interface( mconfig, *this )
{
}

pce_rom_device::pce_rom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, PCE_ROM_STD, "PCE & TG16 Carts", tag, owner, clock, "pce_rom", __FILE__),
						device_pce_cart_interface( mconfig, *this )
{
}

pce_cdsys3_device::pce_cdsys3_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: pce_rom_device(mconfig, PCE_ROM_CDSYS3, "PCE & TG16 CD-System Cart v3.00", tag, owner, clock, "pce_cdsys3", __FILE__)
{
}

pce_populous_device::pce_populous_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: pce_rom_device(mconfig, PCE_ROM_POPULOUS, "PCE Populous Cart", tag, owner, clock, "pce_populous", __FILE__)
{
}

pce_sf2_device::pce_sf2_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: pce_rom_device(mconfig, PCE_ROM_SF2, "PCE Street Fighters 2 Cart", tag, owner, clock, "pce_sf2", __FILE__), m_bank_base(0)
				{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------


void pce_sf2_device::device_start()
{
	save_item(NAME(m_bank_base));
}

void pce_sf2_device::device_reset()
{
	m_bank_base = 0;
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ8_MEMBER(pce_rom_device::read_cart)
{
	int bank = offset / 0x20000;
	return m_rom[rom_bank_map[bank] * 0x20000 + (offset & 0x1ffff)];
}


READ8_MEMBER(pce_cdsys3_device::read_cart)
{
	int bank = offset / 0x20000;
	if (!m_ram.empty() && offset >= 0xd0000)
		return m_ram[offset - 0xd0000];

	return m_rom[rom_bank_map[bank] * 0x20000 + (offset & 0x1ffff)];
}

WRITE8_MEMBER(pce_cdsys3_device::write_cart)
{
	if (!m_ram.empty() && offset >= 0xd0000)
		m_ram[offset - 0xd0000] = data;
}


READ8_MEMBER(pce_populous_device::read_cart)
{
	int bank = offset / 0x20000;
	if (!m_ram.empty() && offset >= 0x80000 && offset < 0x88000)
		return m_ram[offset & 0x7fff];

	return m_rom[rom_bank_map[bank] * 0x20000 + (offset & 0x1ffff)];
}

WRITE8_MEMBER(pce_populous_device::write_cart)
{
	if (!m_ram.empty() && offset >= 0x80000 && offset < 0x88000)
		m_ram[offset & 0x7fff] = data;
}


READ8_MEMBER(pce_sf2_device::read_cart)
{
	if (offset < 0x80000)
		return m_rom[offset];
	else
		return m_rom[0x80000 + m_bank_base * 0x80000 + (offset & 0x7ffff)];
}

WRITE8_MEMBER(pce_sf2_device::write_cart)
{
	if (offset >= 0x1ff0 && offset < 0x1ff4)
		m_bank_base = offset & 3;
}
