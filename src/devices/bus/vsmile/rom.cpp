// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***********************************************************************************************************

 V.Smile cart emulation

 We support standard carts and one with on-board RAM

 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  vsmile_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(VSMILE_ROM_STD, vsmile_rom_device,     "vsmile_rom",     "V.Smile Cart")
DEFINE_DEVICE_TYPE(VSMILE_ROM_RAM, vsmile_rom_ram_device, "vsmile_rom_ram", "V.Smile Cart + RAM")


vsmile_rom_device::vsmile_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_vsmile_cart_interface(mconfig, *this)
{
}

vsmile_rom_device::vsmile_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vsmile_rom_device(mconfig, VSMILE_ROM_STD, tag, owner, clock)
{
}

vsmile_rom_ram_device::vsmile_rom_ram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: vsmile_rom_device(mconfig, type, tag, owner, clock)
{
}

vsmile_rom_ram_device::vsmile_rom_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vsmile_rom_ram_device(mconfig, VSMILE_ROM_RAM, tag, owner, clock)
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void vsmile_rom_device::device_start()
{
}

void vsmile_rom_device::device_reset()
{
}


/*-------------------------------------------------
 Carts with RAM
 -------------------------------------------------*/

READ16_MEMBER(vsmile_rom_ram_device::bank2_r)
{
	if (!m_ram.empty() && offset < m_ram.size())
		return m_ram[offset];
	else    // this cannot actually happen...
		return 0;
}

WRITE16_MEMBER(vsmile_rom_ram_device::bank2_w)
{
	if (!m_ram.empty() && offset < m_ram.size())
		COMBINE_DATA(&m_ram[offset]);
}
