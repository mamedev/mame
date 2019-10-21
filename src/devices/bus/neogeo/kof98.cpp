// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 The King of Fighers '98 cart type

 ***********************************************************************************************************/


#include "emu.h"
#include "kof98.h"


//-------------------------------------------------
//  neogeo_kof98_cart_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NEOGEO_KOF98_CART, neogeo_kof98_cart_device, "neocart_kof98", "Neo Geo KoF 98 Cart")


neogeo_kof98_cart_device::neogeo_kof98_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_rom_device(mconfig, NEOGEO_KOF98_CART, tag, owner, clock),
	m_prot(*this, "kof98_prot")
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_kof98_cart_device::device_start()
{
}

void neogeo_kof98_cart_device::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

void neogeo_kof98_cart_device::device_add_mconfig(machine_config &config)
{
	NG_KOF98_PROT(config, m_prot);
}

void neogeo_kof98_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->decrypt_68k(cpuregion, cpuregion_size);
}
