// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 Nintendo Virtual Boy cart emulation


 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  vboy_rom_device - constructor
//-------------------------------------------------

const device_type VBOY_ROM_STD = &device_creator<vboy_rom_device>;
const device_type VBOY_ROM_EEPROM = &device_creator<vboy_eeprom_device>;


vboy_rom_device::vboy_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_vboy_cart_interface( mconfig, *this )
{
}

vboy_rom_device::vboy_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, VBOY_ROM_STD, "Nintendo Virtual Boy Carts", tag, owner, clock, "vboy_rom", __FILE__),
						device_vboy_cart_interface( mconfig, *this )
{
}

vboy_eeprom_device::vboy_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: vboy_rom_device(mconfig, VBOY_ROM_EEPROM, "Nintendo Virtual Boy Carts + EEPROM", tag, owner, clock, "vboy_eeprom", __FILE__)
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ32_MEMBER(vboy_rom_device::read_cart)
{
	return m_rom[offset & m_rom_mask];
}


READ32_MEMBER(vboy_eeprom_device::read_eeprom)
{
	return m_eeprom[offset];
}


WRITE32_MEMBER(vboy_eeprom_device::write_eeprom)
{
	COMBINE_DATA(&m_eeprom[offset]);
}
