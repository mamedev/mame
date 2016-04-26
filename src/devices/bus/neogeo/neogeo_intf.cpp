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
m_rom(nullptr),
m_fixed(nullptr),
m_sprites(nullptr),
m_sprites_optimized(nullptr),
m_audio(nullptr),
m_ym(nullptr),
m_ymdelta(nullptr),
m_audiocrypt(nullptr)
*/
{
}


//-------------------------------------------------
//  ~device_neogeo_cart_interface - destructor
//-------------------------------------------------

device_neogeo_cart_interface::~device_neogeo_cart_interface()
{
}
