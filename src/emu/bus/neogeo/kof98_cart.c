// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
/***********************************************************************************************************

 NEOGEO ROM cart emulation

 ***********************************************************************************************************/


#include "emu.h"
#include "kof98_cart.h"


//-------------------------------------------------
//  neogeo_kof98_cart - constructor
//-------------------------------------------------

const device_type NEOGEO_KOF98_CART = &device_creator<neogeo_kof98_cart>;


neogeo_kof98_cart::neogeo_kof98_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_neogeo_cart_interface( mconfig, *this ),
						m_banked_cart(*this, "banked_cart"),
						m_kof98_prot(*this, "kof98_prot")
{
}

neogeo_kof98_cart::neogeo_kof98_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock)
					: device_t(mconfig, NEOGEO_KOF98_CART, "NEOGEO KOF98 Cart", tag, owner, clock, "neogeo_rom", __FILE__),
						device_neogeo_cart_interface( mconfig, *this ),
						m_banked_cart(*this, "banked_cart"),
						m_kof98_prot(*this, "kof98_prot")
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_kof98_cart::device_start()
{
}

void neogeo_kof98_cart::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ16_MEMBER(neogeo_kof98_cart::read_rom)
{
	return m_rom[offset];
}

static MACHINE_CONFIG_FRAGMENT( kof98_cart )
	MCFG_NEOGEO_BANKED_CART_ADD("banked_cart")
	MCFG_KOF98_PROT_ADD("kof98_prot")

MACHINE_CONFIG_END

machine_config_constructor neogeo_kof98_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( kof98_cart );
}

void neogeo_kof98_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof98_prot->kof98_decrypt_68k(cpuregion, cpuregion_size);
}

void neogeo_kof98_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);
	m_kof98_prot->install_kof98_protection(maincpu);
}
