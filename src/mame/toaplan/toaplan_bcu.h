// license:BSD-3-Clause
// copyright-holders:Darren Olafson, Quench
/***************************************************************************

Toaplan 'BCU' tilemap generator

***************************************************************************/
#ifndef MAME_TOAPLAN_TOAPLAN_BCU_H
#define MAME_TOAPLAN_TOAPLAN_BCU_H

#pragma once

#include "tilemap.h"

class toaplan_bcu_device : public device_t,
							public device_gfx_interface
{
public:
	toaplan_bcu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T> toaplan_bcu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: toaplan_bcu_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	// configurations
	void set_offset(s32 dx, s32 dy, s32 dx_flip, s32 dy_flip)
	{
		m_dx = dx;
		m_dy = dy;
		m_dx_flipped = dx_flip;
		m_dy_flipped = dy_flip;
	}

	// read/write handlers
	u16 tileram_offs_r();
	void tileram_offs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 tileram_r(offs_t offset);
	void tileram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 scroll_regs_r(offs_t offset);
	void scroll_regs_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void tile_offsets_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void flipscreen_w(u8 data);

	// renderer
	void draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u32 flags, u8 pri = 0, u8 pri_mask = ~0);
	void draw_background(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 flags, u8 pri = 0, u8 pri_mask = ~0);
	void draw_tilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u32 flags, u8 pri = 0, u8 pri_mask = ~0);
	void draw_tilemap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 flags, u8 pri = 0, u8 pri_mask = ~0);

	void host_map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	template <class BitmapClass> void draw_background_base(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect, u32 flags, u8 pri, u8 pri_mask);
	template <class BitmapClass> void draw_tilemap_base(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect, u32 flags, u8 pri, u8 pri_mask);

	template<unsigned Layer> TILE_GET_INFO_MEMBER(get_tile_info);

	void set_scrolls();

	// memory pointers
	memory_share_array_creator<u32, 4> m_vram;

	// internal states
	tilemap_t *m_tilemap[4];

	s32 m_ram_offs;

	s32 m_scrollx[4];
	s32 m_scrolly[4];

	s32 m_offsetx;
	s32 m_offsety;

	s32 m_flipscreen;

	// configurations
	s32 m_dx, m_dy;
	s32 m_dx_flipped, m_dy_flipped;

};

DECLARE_DEVICE_TYPE(TOAPLAN_BCU, toaplan_bcu_device)

#endif // MAME_TOAPLAN_TOAPLAN_BCU_H
