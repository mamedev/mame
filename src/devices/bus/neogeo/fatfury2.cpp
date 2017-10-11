// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 Fatal Fury 2 cart type

 ***********************************************************************************************************/


#include "emu.h"
#include "fatfury2.h"


//-------------------------------------------------
//  neogeo_fatfury2_cart_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NEOGEO_FATFURY2_CART, neogeo_fatfury2_cart_device, "neocart_fatfury2", "Neo Geo Fatal Furty 2 Cart")


neogeo_fatfury2_cart_device::neogeo_fatfury2_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_rom_device(mconfig, NEOGEO_FATFURY2_CART, tag, owner, clock),
	m_prot(*this, "fatfury2_prot")
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_fatfury2_cart_device::device_start()
{
}

void neogeo_fatfury2_cart_device::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

MACHINE_CONFIG_MEMBER( neogeo_fatfury2_cart_device::device_add_mconfig )
	MCFG_FATFURY2_PROT_ADD("fatfury2_prot")
MACHINE_CONFIG_END
