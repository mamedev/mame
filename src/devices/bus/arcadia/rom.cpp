// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 Emerson Arcadia 2001 cart emulation

 Golf carts have the "extra_rom" handler installed at $4000 instead of $2000

 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  arcadia_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(ARCADIA_ROM_STD,  arcadia_rom_device,  "arcadia_rom",  "Emerson Arcadia Standard Carts")
DEFINE_DEVICE_TYPE(ARCADIA_ROM_GOLF, arcadia_golf_device, "arcadia_golf", "Emerson Arcadia Golf Cart")


arcadia_rom_device::arcadia_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock), device_arcadia_cart_interface(mconfig, *this)
{
}

arcadia_rom_device::arcadia_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arcadia_rom_device(mconfig, ARCADIA_ROM_STD, tag, owner, clock)
{
}

arcadia_golf_device::arcadia_golf_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arcadia_rom_device(mconfig, ARCADIA_ROM_GOLF, tag, owner, clock)
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

uint8_t arcadia_rom_device::read_rom(offs_t offset)
{
	if (offset < m_rom_size)
		return m_rom[offset];
	else
		return 0xff;
}


uint8_t arcadia_rom_device::extra_rom(offs_t offset)
{
	if (offset + 0x1000 < m_rom_size)
		return m_rom[offset + 0x1000];
	else
		return 0xff;
}
