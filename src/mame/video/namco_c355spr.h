// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_VIDEO_NAMCO_C355SPR_H
#define MAME_VIDEO_NAMCO_C355SPR_H

#pragma once

#include "screen.h"

class namco_c355spr_device : public device_t, public device_gfx_interface, public device_video_interface
{
public:
	// construction/destruction
	namco_c355spr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_color_base(u16 base) { m_colbase = base; }
	void set_scroll_offsets(int x, int y) { m_scrolloffs[0] = x; m_scrolloffs[1] = y; }
	//void set_ram_words(u32 size) { m_ramsize = size; }
	void set_palxor(int palxor) { m_palxor = palxor; }
	void set_buffer(int buffer) { m_buffer = buffer; }
	void set_external_prifill(bool external) { m_external_prifill = external; }

	u16 spriteram_r(offs_t offset);
	void spriteram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 position_r(offs_t offset);
	void position_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	DECLARE_WRITE_LINE_MEMBER(vblank);

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
	void get_sprites(const rectangle cliprect);
	void copy_sprites(const rectangle cliprect);

	bitmap_ind16 &screen_bitmap() { return m_screenbitmap; }
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_stop() override;

private:

	struct c355_sprite
	{
		bool disable;
		int size;
		rectangle clip;
		int offset;
		int color;
		bool flipx,flipy;
		int tile[16*16];
		int x[16*16],y[16*16];
		int zoomx[16*16],zoomy[16*16];
		int pri;
	};

	// general
	void zdrawgfxzoom(bitmap_ind16 &dest_bmp, const rectangle &clip, gfx_element *gfx, u32 code, u32 color, bool flipx, bool flipy, int sx, int sy, int scalex, int scaley, u8 prival);

	void copybitmap(bitmap_ind16 &dest_bmp, const rectangle &clip, u8 pri);
	void copybitmap(bitmap_rgb32 &dest_bmp, const rectangle &clip, u8 pri);

	// C355 Motion Object Emulation
	// for pal_xor, supply either 0x0 (normal) or 0xf (palette mapping reversed)
	int default_code2tile(int code);

	// C355 Motion Object internals
	void get_single_sprite(const u16 *pSource, c355_sprite *sprite_ptr);
	void get_list(int no, const u16 *pSpriteList16, const u16 *pSpriteTable);
	template<class BitmapClass> void draw_sprites(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect, int pri);

	std::unique_ptr<c355_sprite []> m_spritelist[2];
	const c355_sprite *m_sprite_end[2];
	c355_obj_code2tile_delegate m_code2tile;
	int m_palxor;
	u16 m_position[4];
	std::unique_ptr<u16 []> m_spriteram[2];
	bitmap_ind16 m_tempbitmap;
	bitmap_ind16 m_screenbitmap;

	int m_scrolloffs[2];
	//u32 m_ramsize;
	int m_buffer;
	bool m_external_prifill;

	required_memory_region m_gfx_region;
	u16 m_colbase;
};

// device type definition
DECLARE_DEVICE_TYPE(NAMCO_C355SPR, namco_c355spr_device)

#endif // MAME_VIDEO_NAMCO_C355SPR_H
