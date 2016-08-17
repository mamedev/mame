// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 Metal Slug X cart type

 ***********************************************************************************************************/


#include "emu.h"
#include "mslugx.h"


//-------------------------------------------------
//  neogeo_mslugx_cart - constructor
//-------------------------------------------------

const device_type NEOGEO_MSLUGX_CART = &device_creator<neogeo_mslugx_cart>;


neogeo_mslugx_cart::neogeo_mslugx_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock) :
	neogeo_rom_device(mconfig, NEOGEO_MSLUGX_CART, "Neo Geo Metal Slug X Cart", tag, owner, clock, "neocart_mslugx", __FILE__),
	m_prot(*this, "mslugx_prot")
{}


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

static MACHINE_CONFIG_FRAGMENT( mslugx_cart )
	MCFG_MSLUGX_PROT_ADD("mslugx_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_mslugx_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( mslugx_cart );
}
