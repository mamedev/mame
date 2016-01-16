// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
/***********************************************************************************************************

 NEOGEO ROM cart emulation

 ***********************************************************************************************************/


#include "emu.h"
#include "mslugx_cart.h"


//-------------------------------------------------
//  neogeo_mslugx_cart - constructor
//-------------------------------------------------

const device_type NEOGEO_MSLUGX_CART = &device_creator<neogeo_mslugx_cart>;


neogeo_mslugx_cart::neogeo_mslugx_cart(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT16 clock, std::string shortname, std::string source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_neogeo_cart_interface( mconfig, *this ),
						m_banked_cart(*this, "banked_cart"),
						m_mslugx_prot(*this, "mslugx_prot")
{
}

neogeo_mslugx_cart::neogeo_mslugx_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT16 clock)
					: device_t(mconfig, NEOGEO_MSLUGX_CART, "NEOGEO Metal Slug X Cart", tag, owner, clock, "neogeo_rom", __FILE__),
						device_neogeo_cart_interface( mconfig, *this ),
						m_banked_cart(*this, "banked_cart"),
						m_mslugx_prot(*this, "mslugx_prot")
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_mslugx_cart::device_start()
{
}

void neogeo_mslugx_cart::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ16_MEMBER(neogeo_mslugx_cart::read_rom)
{
	return m_rom[offset];
}

static MACHINE_CONFIG_FRAGMENT( mslugx_cart )
	MCFG_NEOGEO_BANKED_CART_ADD("banked_cart")
	MCFG_MSLUGX_PROT_ADD("mslugx_prot")

MACHINE_CONFIG_END

machine_config_constructor neogeo_mslugx_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( mslugx_cart );
}

void neogeo_mslugx_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);
	m_mslugx_prot->mslugx_install_protection(maincpu);
}
