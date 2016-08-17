// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 M5 cart emulation


 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  m5_rom_device - constructor
//-------------------------------------------------

const device_type M5_ROM_STD = &device_creator<m5_rom_device>;
const device_type M5_ROM_RAM = &device_creator<m5_ram_device>;


m5_rom_device::m5_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_m5_cart_interface( mconfig, *this )
{
}

m5_rom_device::m5_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, M5_ROM_STD, "M5 Standard ROM Carts", tag, owner, clock, "m5_rom", __FILE__),
						device_m5_cart_interface( mconfig, *this )
{
}


m5_ram_device::m5_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: m5_rom_device(mconfig, M5_ROM_RAM, "M5 Expansion memory cart", tag, owner, clock, "m5_ram", __FILE__)
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ8_MEMBER(m5_rom_device::read_rom)
{
	if (offset < m_rom_size)
		return m_rom[offset];
	else
		return 0xff;
}

READ8_MEMBER(m5_ram_device::read_ram)
{
	return m_ram[offset];
}

WRITE8_MEMBER(m5_ram_device::write_ram)
{
	m_ram[offset] = data;
}
