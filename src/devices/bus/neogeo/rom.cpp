// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
/***********************************************************************************************************

 NEOGEO ROM cart emulation

 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  neogeo_rom_device - constructor
//-------------------------------------------------

const device_type NEOGEO_ROM = &device_creator<neogeo_rom_device>;


neogeo_rom_device::neogeo_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_neogeo_cart_interface( mconfig, *this ),
						m_banked_cart(*this, "banked_cart")
{
}

neogeo_rom_device::neogeo_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock)
					: device_t(mconfig, NEOGEO_ROM, "NEOGEO ROM Carts", tag, owner, clock, "neogeo_rom", __FILE__),
						device_neogeo_cart_interface( mconfig, *this ),
						m_banked_cart(*this, "banked_cart")
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_rom_device::device_start()
{
}

void neogeo_rom_device::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ16_MEMBER(neogeo_rom_device::read_rom)
{
	return m_rom[offset];
}

static MACHINE_CONFIG_FRAGMENT( banked_cart )
	MCFG_NEOGEO_BANKED_CART_ADD("banked_cart")
MACHINE_CONFIG_END

machine_config_constructor neogeo_rom_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( banked_cart );
}

void neogeo_rom_device::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);
}
