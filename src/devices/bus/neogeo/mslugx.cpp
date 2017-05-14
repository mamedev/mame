// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 Metal Slug X cart type

 ***********************************************************************************************************/


#include "emu.h"
#include "mslugx.h"


//-------------------------------------------------
//  neogeo_mslugx_cart_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NEOGEO_MSLUGX_CART, neogeo_mslugx_cart_device, "neocart_mslugx", "Neo Geo Metal Slug X Cart")


neogeo_mslugx_cart_device::neogeo_mslugx_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_rom_device(mconfig, NEOGEO_MSLUGX_CART, tag, owner, clock),
	m_prot(*this, "mslugx_prot")
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_mslugx_cart_device::device_start()
{
}

void neogeo_mslugx_cart_device::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

static MACHINE_CONFIG_FRAGMENT( mslugx_cart )
	MCFG_MSLUGX_PROT_ADD("mslugx_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_mslugx_cart_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( mslugx_cart );
}
