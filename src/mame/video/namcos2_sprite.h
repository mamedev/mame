// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_VIDEO_NAMCOS2_SPRITE_H
#define MAME_VIDEO_NAMCOS2_SPRITE_H

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

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, int control );
	void draw_sprites_metalhawk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri );

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// general
	void zdrawgfxzoom(screen_device &screen, bitmap_ind16 &dest_bmp, const rectangle &clip, gfx_element *gfx, uint32_t code, uint32_t color, int flipx, int flipy, int sx, int sy, int scalex, int scaley, int zpos);
	void zdrawgfxzoom(screen_device &screen, bitmap_rgb32 &dest_bmp, const rectangle &clip, gfx_element *gfx, uint32_t code, uint32_t color, int flipx, int flipy, int sx, int sy, int scalex, int scaley, int zpos);

	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint16_t> m_spriteram;
};

// device type definition
DECLARE_DEVICE_TYPE(NAMCOS2_SPRITE, namcos2_sprite_device)

#endif // MAME_VIDEO_NAMCOS2_SPRITE_H

