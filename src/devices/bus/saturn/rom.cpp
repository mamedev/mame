// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

 Saturn ROM cart emulation

 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  saturn_rom_device - constructor
//-------------------------------------------------

const device_type SATURN_ROM = &device_creator<saturn_rom_device>;


saturn_rom_device::saturn_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_sat_cart_interface( mconfig, *this )
{
}

saturn_rom_device::saturn_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
					: device_t(mconfig, SATURN_ROM, "Saturn ROM Carts", tag, owner, clock, "sat_rom", __FILE__),
						device_sat_cart_interface( mconfig, *this )
{
	m_cart_type = 0xff; // actually not clear if ROM carts have a type ID like DRAM/BRAM carts
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void saturn_rom_device::device_start()
{
}

void saturn_rom_device::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

uint32_t saturn_rom_device::read_rom(address_space &space, offs_t offset, uint32_t mem_mask)
{
	return m_rom[offset & (m_rom_size/4 - 1)];
}
