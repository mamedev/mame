// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood

#include "emu.h"
#include "neogeo_intf.h"

//-------------------------------------------------
//  device_neogeo_cart_interface - constructor
//-------------------------------------------------

device_neogeo_cart_interface::device_neogeo_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device), 
		m_sprite_gfx_address_mask(0)
/*
m_rom(NULL),
m_fixed(NULL),
m_sprites(NULL),
m_sprites_optimized(NULL),
m_audio(NULL),
m_ym(NULL),
m_ymdelta(NULL),
m_audiocrypt(NULL)
*/
{
}


//-------------------------------------------------
//  ~device_neogeo_cart_interface - destructor
//-------------------------------------------------

device_neogeo_cart_interface::~device_neogeo_cart_interface()
{
}
