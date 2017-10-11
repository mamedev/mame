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

DEFINE_DEVICE_TYPE(VBOY_ROM_STD,    vboy_rom_device,    "vboy_rom",    "Nintendo Virtual Boy Carts")
DEFINE_DEVICE_TYPE(VBOY_ROM_EEPROM, vboy_eeprom_device, "vboy_eeprom", "Nintendo Virtual Boy Carts + EEPROM")


vboy_rom_device::vboy_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock), device_vboy_cart_interface(mconfig, *this)
{
}

vboy_rom_device::vboy_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vboy_rom_device(mconfig, VBOY_ROM_STD, tag, owner, clock)
{
}

vboy_eeprom_device::vboy_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vboy_rom_device(mconfig, VBOY_ROM_EEPROM, tag, owner, clock)
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
