// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_VIDEO_NAMCO_C355SPR_H
#define MAME_VIDEO_NAMCO_C355SPR_H

#pragma once

#include "screen.h"
#include "emupal.h"

class namco_c355spr_device : public device_t
{
public:
	// construction/destruction
	namco_c355spr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T> void set_palette_tag(T &&tag) { m_palette.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
	void set_is_namcofl(bool state) { m_is_namcofl = state; }
	//void set_ram_words(uint32_t size) { m_ramsize = size; }
	void set_palxor(int palxor) { m_palxor = palxor; }
	void set_gfxregion(int region) { m_gfx_region = region; }

	DECLARE_READ16_MEMBER( spriteram_r );
	DECLARE_WRITE16_MEMBER( spriteram_w );
	DECLARE_READ16_MEMBER( position_r );
	DECLARE_WRITE16_MEMBER( position_w );

	typedef delegate<int (int)> c355_obj_code2tile_delegate;
	void set_tile_callback(c355_obj_code2tile_delegate cb)
	{
		if (!cb.isnull())
			m_code2tile = cb;
		else
			m_code2tile = c355_obj_code2tile_delegate(&namco_c355spr_device::default_code2tile, this);
	}

	void draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri);
	void draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri);


protected:
	// device-level overrides
	virtual void device_start() override;

private:

	// general
	void zdrawgfxzoom(screen_device &screen, bitmap_ind16 &dest_bmp, const rectangle &clip, gfx_element *gfx, uint32_t code, uint32_t color, int flipx, int flipy, int sx, int sy, int scalex, int scaley, int zpos);
	void zdrawgfxzoom(screen_device &screen, bitmap_rgb32 &dest_bmp, const rectangle &clip, gfx_element *gfx, uint32_t code, uint32_t color, int flipx, int flipy, int sx, int sy, int scalex, int scaley, int zpos);

	// C355 Motion Object Emulation
	// for pal_xor, supply either 0x0 (normal) or 0xf (palette mapping reversed)
	int default_code2tile(int code);

	// C355 Motion Object internals
	template<class _BitmapClass>
	void draw_sprite(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, const uint16_t *pSource, int pri, int zpos);
	template<class _BitmapClass>
	void draw_list(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, int pri, const uint16_t *pSpriteList16, const uint16_t *pSpriteTable);

	c355_obj_code2tile_delegate m_code2tile;
	int m_gfx_region;
	int m_palxor;
	uint16_t m_position[4];
	uint16_t m_spriteram[0x20000/2];
	//std::vector<uint16_t> m_spriteram;

	bool m_is_namcofl;
	//uint32_t m_ramsize;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

// device type definition
DECLARE_DEVICE_TYPE(NAMCO_C355SPR, namco_c355spr_device)

#endif // MAME_VIDEO_NAMCO_C355SPR_H

