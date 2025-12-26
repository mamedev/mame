// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_NAMCO_NAMCOS2_SPRITE_H
#define MAME_NAMCO_NAMCOS2_SPRITE_H

#pragma once

#include "screen.h"
#include "emupal.h"

class namcos2_sprite_device : public device_t, public device_gfx_interface
{
public:
	using ns2_priority_delegate = device_delegate<u32 (u32 pri)>;
	using ns2_mix_delegate = device_delegate<bool (u16 &dest, u8 &destpri, u16 colbase, u16 src, u32 primask)>;

	// construction/destruction
	namcos2_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T> namcos2_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: namcos2_sprite_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	template <typename T> void set_spriteram_tag(T &&tag) { m_spriteram.set_tag(std::forward<T>(tag)); }

	template <typename... T> void set_priority_callback(T &&... args) { m_pri_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_mix_callback(T &&... args) { m_mix_cb.set(std::forward<T>(args)...); }

	virtual void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int control);

protected:
	namcos2_sprite_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// general
	void zdrawgfxzoom(screen_device &screen, bitmap_ind16 &dest_bmp, const rectangle &clip, gfx_element *gfx, u32 code, u32 color, bool flipx, bool flipy, int sx, int sy, int scalex, int scaley, int primask);
	void zdrawgfxzoom(screen_device &screen, bitmap_rgb32 &dest_bmp, const rectangle &clip, gfx_element *gfx, u32 code, u32 color, bool flipx, bool flipy, int sx, int sy, int scalex, int scaley, int primask);

	virtual void get_tilenum_and_size(const u16 word0, const u16 word1, u32 &sprn, bool &is_32);

	required_shared_ptr<u16> m_spriteram;

	ns2_priority_delegate m_pri_cb;
	ns2_mix_delegate m_mix_cb;
};

class namcos2_sprite_finallap_device : public namcos2_sprite_device
{
public:
	// construction/destruction
	namcos2_sprite_finallap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T> namcos2_sprite_finallap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: namcos2_sprite_finallap_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

protected:
	virtual void get_tilenum_and_size(const u16 word0, const u16 word1, u32& sprn, bool& is_32) override;
};


class namcos2_sprite_metalhawk_device : public namcos2_sprite_device
{
public:
	// construction/destruction
	namcos2_sprite_metalhawk_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T> namcos2_sprite_metalhawk_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: namcos2_sprite_metalhawk_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	virtual void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int control) override;
};



// device type definition
DECLARE_DEVICE_TYPE(NAMCOS2_SPRITE, namcos2_sprite_device)
DECLARE_DEVICE_TYPE(NAMCOS2_SPRITE_FINALLAP, namcos2_sprite_finallap_device)
DECLARE_DEVICE_TYPE(NAMCOS2_SPRITE_METALHAWK, namcos2_sprite_metalhawk_device)

#endif // MAME_NAMCO_NAMCOS2_SPRITE_H
