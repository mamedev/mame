// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_NAMCO_NAMCOS2_SPRITE_H
#define MAME_NAMCO_NAMCOS2_SPRITE_H

#pragma once

#include "screen.h"
#include "emupal.h"

class namcos2_sprite_device : public device_t
{
public:
	// construction/destruction
	namcos2_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_spriteram_tag(T &&tag) { m_spriteram.set_tag(std::forward<T>(tag)); }

	virtual void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, int control );

protected:
	namcos2_sprite_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// general
	void zdrawgfxzoom(screen_device &screen, bitmap_ind16 &dest_bmp, const rectangle &clip, gfx_element *gfx, u32 code, u32 color, bool flipx, bool flipy, int sx, int sy, int scalex, int scaley, int zpos);
	void zdrawgfxzoom(screen_device &screen, bitmap_rgb32 &dest_bmp, const rectangle &clip, gfx_element *gfx, u32 code, u32 color, bool flipx, bool flipy, int sx, int sy, int scalex, int scaley, int zpos);

	virtual void get_tilenum_and_size(const u16 word0, const u16 word1, u32 &sprn, bool &is_32);

	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<u16> m_spriteram;
};

class namcos2_sprite_finallap_device : public namcos2_sprite_device
{
public:
	// construction/destruction
	namcos2_sprite_finallap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void get_tilenum_and_size(const u16 word0, const u16 word1, u32& sprn, bool& is_32) override;
};


class namcos2_sprite_metalhawk_device : public namcos2_sprite_device
{
public:
	// construction/destruction
	namcos2_sprite_metalhawk_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, int control ) override;
};



// device type definition
DECLARE_DEVICE_TYPE(NAMCOS2_SPRITE, namcos2_sprite_device)
DECLARE_DEVICE_TYPE(NAMCOS2_SPRITE_FINALLAP, namcos2_sprite_finallap_device)
DECLARE_DEVICE_TYPE(NAMCOS2_SPRITE_METALHAWK, namcos2_sprite_metalhawk_device)

#endif // MAME_NAMCO_NAMCOS2_SPRITE_H
