// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
/***********************************************************************************************************

 NEOGEO ROM cart emulation

 ***********************************************************************************************************/


#include "emu.h"
#include "fatfury2_cart.h"


//-------------------------------------------------
//  neogeo_fatfury2_cart - constructor
//-------------------------------------------------

const device_type NEOGEO_FATFURY2_CART = &device_creator<neogeo_fatfury2_cart>;


neogeo_fatfury2_cart::neogeo_fatfury2_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_neogeo_cart_interface( mconfig, *this ),
						m_banked_cart(*this, "banked_cart"),
						m_fatfury2_prot(*this, "fatfury2_prot")
{
}

neogeo_fatfury2_cart::neogeo_fatfury2_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock)
					: device_t(mconfig, NEOGEO_FATFURY2_CART, "NEOGEO Fatal Fury 2 Cart", tag, owner, clock, "neogeo_rom", __FILE__),
						device_neogeo_cart_interface( mconfig, *this ),
						m_banked_cart(*this, "banked_cart"),
						m_fatfury2_prot(*this, "fatfury2_prot")
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_fatfury2_cart::device_start()
{
}

void neogeo_fatfury2_cart::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ16_MEMBER(neogeo_fatfury2_cart::read_rom)
{
	return m_rom[offset];
}

static MACHINE_CONFIG_FRAGMENT( fatfury2_cart )
	MCFG_NEOGEO_BANKED_CART_ADD("banked_cart")
	MCFG_FATFURY2_PROT_ADD("fatfury2_prot")

MACHINE_CONFIG_END

machine_config_constructor neogeo_fatfury2_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( fatfury2_cart );
}

void neogeo_fatfury2_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);
	m_fatfury2_prot->fatfury2_install_protection(maincpu, m_banked_cart);
}
