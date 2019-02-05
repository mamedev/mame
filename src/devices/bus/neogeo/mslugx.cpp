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

void neogeo_mslugx_cart_device::device_add_mconfig(machine_config &config)
{
	NG_MSLUGX_PROT(config, m_prot);
}
