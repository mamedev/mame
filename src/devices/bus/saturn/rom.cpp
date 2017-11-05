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

DEFINE_DEVICE_TYPE(SATURN_ROM, saturn_rom_device, "sat_rom", "Saturn ROM Carts")


saturn_rom_device::saturn_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int cart_type)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sat_cart_interface(mconfig, *this, cart_type)
{
}

saturn_rom_device::saturn_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: saturn_rom_device(mconfig, SATURN_ROM, tag, owner, clock, 0xff) // actually not clear if ROM carts have a type ID like DRAM/BRAM carts
{
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

READ32_MEMBER(saturn_rom_device::read_rom)
{
	return m_rom[offset & (m_rom_size/4 - 1)];
}
