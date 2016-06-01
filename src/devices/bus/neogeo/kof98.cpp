// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 The King of Fighers '98 cart type

 ***********************************************************************************************************/


#include "emu.h"
#include "kof98.h"


//-------------------------------------------------
//  neogeo_kof98_cart - constructor
//-------------------------------------------------

const device_type NEOGEO_KOF98_CART = &device_creator<neogeo_kof98_cart>;


neogeo_kof98_cart::neogeo_kof98_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock) :
	neogeo_rom_device(mconfig, NEOGEO_KOF98_CART, "Neo Geo KOF 98 Cart", tag, owner, clock, "neocart_kof98", __FILE__),
	m_prot(*this, "kof98_prot")
{}


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

static MACHINE_CONFIG_FRAGMENT( kof98_cart )
	MCFG_KOF98_PROT_ADD("kof98_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_kof98_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( kof98_cart );
}

void neogeo_kof98_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->decrypt_68k(cpuregion, cpuregion_size);
}
