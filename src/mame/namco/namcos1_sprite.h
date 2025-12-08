// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Nicola Salmoria, Ernesto Corvi

#ifndef MAME_NAMCO_NAMCOS1_SPRITE_H
#define MAME_NAMCO_NAMCOS1_SPRITE_H

#pragma once

#include "screen.h"

#include <utility>


class namcos1_sprite_device : public device_t, public device_gfx_interface
{
public:
	// delegates
	using shadow_delegate = device_delegate<std::pair<bool, u8 const *> (u8 color)>;
	using pri_delegate = device_delegate<u32 (u8 attr1, u8 attr2)>;
	using gfxbank_delegate = device_delegate<u32 (u32 code, u32 bank)>;

	// construction/destruction
	namcos1_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T> namcos1_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: namcos1_sprite_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	// configuration
	template <typename... T> void set_shadow_callback(T &&... args) { m_shadow_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_pri_callback(T &&... args) { m_pri_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_gfxbank_callback(T &&... args) { m_gfxbank_cb.set(std::forward<T>(args)...); }
	auto flip_callback() { return m_flip_cb.bind(); }

	void spriteram_map(address_map &map);

	u8 spriteram_r(offs_t offset);
	void spriteram_w(offs_t offset, u8 data);

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void copy_sprites();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	memory_share_creator<u8> m_spriteram;
	shadow_delegate m_shadow_cb;
	pri_delegate m_pri_cb;
	gfxbank_delegate m_gfxbank_cb;
	devcb_write_line m_flip_cb;

	bool m_copy_sprites;
	bool m_flip_screen;
};

// device type declaration
DECLARE_DEVICE_TYPE(NAMCOS1_SPRITE, namcos1_sprite_device)

#endif // MAME_NAMCO_NAMCOS1_SPRITE_H
