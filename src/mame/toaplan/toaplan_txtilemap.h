// license:BSD-3-Clause
// copyright-holders:Quench, David Haywood
/***************************************************************************

Common Toaplan text tilemap generator usally paired with GP9001

***************************************************************************/
#ifndef MAME_TOAPLAN_TOAPLAN_TXTILEMAP_H
#define MAME_TOAPLAN_TOAPLAN_TXTILEMAP_H

#pragma once

#include "tilemap.h"

class toaplan_txtilemap_device : public device_t, public device_gfx_interface
{
public:
	toaplan_txtilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T> toaplan_txtilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: toaplan_txtilemap_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	// configurations
	void set_offset(int dx, int dy, int dx_flipped, int dy_flipped)
	{
		m_dx = dx;
		m_dy = dy;
		m_dx_flipped = dx_flipped;
		m_dy_flipped = dy_flipped;
	}

	// read/write handlers
	u16 videoram_r(offs_t offset);
	u16 linescroll_r(offs_t offset);
	u16 lineselect_r(offs_t offset);
	void videoram_w(offs_t offset, u16 data, u16 mem_mask);
	void linescroll_w(offs_t offset, u16 data, u16 mem_mask);
	void lineselect_w(offs_t offset, u16 data, u16 mem_mask);

	// renderer
	void draw_tilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_tilemap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_tilemap_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_tilemap_bootleg(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	template <class BitmapClass> void draw_tilemap_base(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect);
	template <class BitmapClass> void draw_tilemap_bootleg_base(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect);

	TILE_GET_INFO_MEMBER(get_tile_info);

	memory_share_creator<u16> m_videoram;
	memory_share_creator<u16> m_linescroll;
	memory_share_creator<u16> m_lineselect;

	tilemap_t *m_tilemap = nullptr;

	int m_dx, m_dy;
	int m_dx_flipped, m_dy_flipped;
};

DECLARE_DEVICE_TYPE(TOAPLAN_TXTILEMAP, toaplan_txtilemap_device)

#endif // MAME_TOAPLAN_TOAPLAN_TXTILEMAP_H
