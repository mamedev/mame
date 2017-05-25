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

static MACHINE_CONFIG_START( fatfury2_cart )
	MCFG_FATFURY2_PROT_ADD("fatfury2_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_fatfury2_cart_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( fatfury2_cart );
}
