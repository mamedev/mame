// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
#ifndef MAME_KANEKO_KANEKO_TMAP_H
#define MAME_KANEKO_KANEKO_TMAP_H

#pragma once

#include "tilemap.h"

class kaneko_view2_tilemap_device : public device_t, public device_gfx_interface
{
public:
	typedef device_delegate<void (u8, u32*)> view2_cb_delegate;

	kaneko_view2_tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: kaneko_view2_tilemap_device(mconfig, tag, owner, (u32)0)
	{
	}

	kaneko_view2_tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	void set_colbase(u16 base) { m_colbase = base; }
	void set_offset(int dx, int dy, int xdim, int ydim)
	{
		m_dx = dx;
		m_dy = dy;
		m_xdim = xdim;
		m_ydim = ydim;
	}
	void set_invert_flip(int invert_flip) { m_invert_flip = invert_flip; } // for fantasia (bootleg)

	template <typename... T> void set_tile_callback(T &&... args) { m_view2_cb.set(std::forward<T>(args)...); }

	void vram_w(int _N_, offs_t offset, u16 data, u16 mem_mask = u16(~0));

	// call to do the rendering etc.
	template<class BitmapClass>
	void prepare_common(BitmapClass &bitmap, const rectangle &cliprect);
	template<class BitmapClass>
	void render_tilemap_common(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect, int pri);
	template<class BitmapClass>
	void render_tilemap_alt_common(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect, int pri, int v2pri);

	void prepare(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void prepare(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void render_tilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri);
	void render_tilemap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri);
	void render_tilemap_alt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, int v2pri);
	void render_tilemap_alt(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri, int v2pri);


	// access
	void vram_map(address_map &map) ATTR_COLD;

	u16 regs_r(offs_t offset);
	void regs_w(offs_t offset, u16 data, u16 mem_mask = u16(~0));

	void vram_0_w(offs_t offset, u16 data, u16 mem_mask = u16(~0));
	void vram_1_w(offs_t offset, u16 data, u16 mem_mask = u16(~0));

	u16 vram_0_r(offs_t offset);
	u16 vram_1_r(offs_t offset);

	void scroll_0_w(offs_t offset, u16 data, u16 mem_mask = u16(~0));
	void scroll_1_w(offs_t offset, u16 data, u16 mem_mask = u16(~0));

	u16 scroll_0_r(offs_t offset);
	u16 scroll_1_r(offs_t offset);

	void mark_layer_dirty(u8 Layer);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	template<unsigned Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	required_shared_ptr_array<u16, 2> m_vram;
	required_shared_ptr_array<u16, 2> m_vscroll;

	// set when creating device
	required_memory_region m_gfxrom;
	u16 m_colbase;
	int m_dx, m_dy, m_xdim, m_ydim;
	int m_invert_flip;

	view2_cb_delegate   m_view2_cb;
	std::unique_ptr<u16[]> m_regs;
	tilemap_t* m_tmap[2]{};
};


DECLARE_DEVICE_TYPE(KANEKO_TMAP, kaneko_view2_tilemap_device)

#endif // MAME_KANEKO_KANEKO_TMAP_H
