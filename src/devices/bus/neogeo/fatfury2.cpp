// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************
 
 Neo Geo cart emulation
 Fatal Fury 2 cart type
 
 ***********************************************************************************************************/


#include "emu.h"
#include "fatfury2.h"


//-------------------------------------------------
//  neogeo_fatfury2_cart - constructor
//-------------------------------------------------

const device_type NEOGEO_FATFURY2_CART = &device_creator<neogeo_fatfury2_cart>;


neogeo_fatfury2_cart::neogeo_fatfury2_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock) :
	neogeo_rom_device(mconfig, NEOGEO_FATFURY2_CART, "Neo Geo Fatal Fury 2 Cart", tag, owner, clock, "neocart_fatfury2", __FILE__),
	m_prot(*this, "fatfury2_prot")
{}


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

static MACHINE_CONFIG_FRAGMENT( fatfury2_cart )
	MCFG_FATFURY2_PROT_ADD("fatfury2_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_fatfury2_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( fatfury2_cart );
}

