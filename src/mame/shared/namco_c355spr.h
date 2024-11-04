// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_SHARED_NAMCO_C355SPR_H
#define MAME_SHARED_NAMCO_C355SPR_H

#pragma once

#include "screen.h"

class namco_c355spr_device : public device_t, public device_gfx_interface, public device_video_interface
{
public:
	// construction/destruction
	namco_c355spr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void set_color_base(u16 base) { m_colbase = base; }
	void set_scroll_offsets(int x, int y) { m_scrolloffs[0] = x; m_scrolloffs[1] = y; }
	//void set_ram_words(u32 size) { m_ramsize = size; }
	void set_palxor(int palxor) { m_palxor = palxor; }
	void set_buffer(int buffer) { m_buffer = buffer; }
	void set_external_prifill(bool external) { m_external_prifill = external; }
	void set_colors(int colors) { m_colors = colors; }
	void set_granularity(int granularity) { m_granularity = granularity; }
	void set_draw_2_lists(bool draw_2_lists) { m_draw_2_lists = draw_2_lists; }

	// the Namco code currently requires us to allocate memory in the device, the Data East hookup uses access callbacks
	void set_device_allocates_spriteram_and_bitmaps(bool allocate_memory) { m_device_allocates_spriteram_and_bitmaps = allocate_memory;  }


	template <typename... T> void set_priority_callback(T &&... args) { m_pri_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_read_spritetile(T &&... args) { m_read_spritetile.set(std::forward<T>(args)...); }
	template <typename... T> void set_read_spriteformat(T &&... args) { m_read_spriteformat.set(std::forward<T>(args)...); }
	template <typename... T> void set_read_spritetable(T &&... args) { m_read_spritetable.set(std::forward<T>(args)...); }
	template <typename... T> void set_read_cliptable(T &&... args) { m_read_cliptable.set(std::forward<T>(args)...); }
	template <typename... T> void set_read_spritelist(T &&... args) { m_read_spritelist.set(std::forward<T>(args)...); }

	u16 spriteram_r(offs_t offset);
	void spriteram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 position_r(offs_t offset);
	void position_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void vblank(int state);

	typedef delegate<int (int)> c355_obj_code2tile_delegate;
	typedef device_delegate<u16(int, u8)> c355_obj_entry_attr_delegate;
	typedef device_delegate<u16(int, u8, int)> c355_obj_entry_attr_which_delegate;
	typedef device_delegate<u16(int)> c355_obj_entry_delegate;
	typedef device_delegate<u16(int, int)> c355_obj_entry_which_delegate;
	typedef device_delegate<int(int)> c355_priority_delegate;

	void set_tile_callback(c355_obj_code2tile_delegate cb)
	{
		if (!cb.isnull())
			m_code2tile = cb;
		else
			m_code2tile = c355_obj_code2tile_delegate(&namco_c355spr_device::default_code2tile, this);
	}


	void draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri);
	void draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri);

	void draw_dg(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &pri_bitmap, bitmap_rgb32 &temp_bitmap);

	void build_sprite_list_and_render_sprites(const rectangle cliprect);

	template<class BitmapClass>
	void render_sprites(const rectangle cliprect, bitmap_ind8 *pri_bitmap, BitmapClass &temp_bitmap, int alt_precision);

	void clear_screen_bitmap() { m_screenbitmap.fill(0xffff); }
	void clear_screen_bitmap(const rectangle cliprect) { m_screenbitmap.fill(0xffff, cliprect); }
	bitmap_ind16 &screen_bitmap() { return m_screenbitmap; }

protected:
	namco_c355spr_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock = 0);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;

	c355_obj_code2tile_delegate m_code2tile;
	c355_priority_delegate m_pri_cb;
	c355_obj_entry_delegate m_read_spritetile;
	c355_obj_entry_attr_delegate m_read_spriteformat;
	c355_obj_entry_attr_which_delegate m_read_spritetable;
	c355_obj_entry_attr_delegate m_read_cliptable;
	c355_obj_entry_which_delegate m_read_spritelist;

	u16 read_spritetile(int entry);
	u16 read_spriteformat(int entry, u8 attr);
	u16 read_spritetable(int entry, u8 attr, int whichlist);
	u16 read_cliptable(int entry, u8 attr);
	u16 read_spritelist(int entry, int whichlist);

	int default_priority(int pal_pri) { return ((pal_pri >> 4) & 0xf); }

	// general
	template<class BitmapClass>
	void zdrawgfxzoom(
		BitmapClass *dest_bmp, const rectangle &clip, gfx_element *gfx,
		u32 code, u32 color,
		bool flipx, bool flipy,
		int sx, int sy,
		int scalex, int scaley,
		u8 prival,
		bitmap_ind8 *pri_buffer,
		int sprite_screen_width, int sprite_screen_height,
		bitmap_ind8 *pri_bitmap);

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

	std::unique_ptr<c355_sprite []> m_spritelist[2];
	const c355_sprite *m_sprite_end[2]{};
	int m_palxor;
	u16 m_position[4];
	std::unique_ptr<u16 []> m_spriteram[2];
	bitmap_ind16 m_renderbitmap;
	bitmap_ind16 m_screenbitmap;

	void build_sprite_list(int no);

private:

	void copybitmap(bitmap_ind16 &dest_bmp, const rectangle &clip, u8 pri);
	void copybitmap(bitmap_rgb32 &dest_bmp, const rectangle &clip, u8 pri);

	void copybitmap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &pri_bitmap, bitmap_rgb32 &temp_bitmap);

	// C355 Motion Object Emulation
	// for pal_xor, supply either 0x0 (normal) or 0xf (palette mapping reversed)
	int default_code2tile(int code);

	// C355 Motion Object internals
	void get_single_sprite(u16 which, c355_sprite *sprite_ptr, int no);
	template<class BitmapClass> void draw_sprites(BitmapClass &bitmap, const rectangle &cliprect, int pri);


	int m_scrolloffs[2];
	//u32 m_ramsize;
	int m_buffer;
	bool m_external_prifill;

	required_memory_region m_gfx_region;
	u16 m_colbase;
	int m_colors;
	int m_granularity;
	bool m_draw_2_lists;
	bool m_device_allocates_spriteram_and_bitmaps;
};

// device type definition
DECLARE_DEVICE_TYPE(NAMCO_C355SPR, namco_c355spr_device)

#endif // MAME_SHARED_NAMCO_C355SPR_H
