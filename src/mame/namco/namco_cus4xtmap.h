// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Nicola Salmoria

#ifndef MAME_NAMCO_NAMCO_CUS4XTMAP_H
#define MAME_NAMCO_NAMCO_CUS4XTMAP_H

#pragma once

#include "screen.h"
#include "tilemap.h"

class namco_cus4xtmap_device : public device_t, public device_gfx_interface
{
public:
	using tile_delegate = device_delegate<void (u8 layer, u8 &gfxno, u32 &code)>;

	// construction/destruction
	namco_cus4xtmap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T> namco_cus4xtmap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: namco_cus4xtmap_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	void set_offset(int xoffs, int flipped_xoffs, int yoffs, int flipped_yoffs)
	{
		m_xoffs = xoffs;
		m_yoffs = yoffs;
		m_flipped_xoffs = flipped_xoffs;
		m_flipped_yoffs = flipped_yoffs;
	}

	template <typename... T> void set_tile_callback(T &&... args) { m_tile_cb.set(std::forward<T>(args)...); }

	// handlers
	void vram_w(offs_t offset, u8 data, u8 mem_mask = ~0);
	u8 vram_r(offs_t offset);
	template <unsigned Layer> void scroll_w(offs_t offset, u8 data)
	{
		scroll_set(Layer, offset, data);
	}
	u16 xscroll_r(offs_t layer) { return m_xscroll[layer]; }

	void mark_all_dirty();
	void init_scroll(bool flip);
	void draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u8 layer, u32 flags, u8 prival = 0, u8 primask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	template <unsigned Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	void set_tilemap_videoram(int offset, u16 newword);
	void scroll_set(int layer, u8 offset, u8 data);

	tile_delegate m_tile_cb;
	memory_share_creator<u8> m_vram;

	tilemap_t *m_tilemap[2];
	u16 m_xscroll[2];
	u8 m_yscroll[2];

	int m_xoffs, m_yoffs;
	int m_flipped_xoffs, m_flipped_yoffs;
};

// device type definition
DECLARE_DEVICE_TYPE(NAMCO_CUS4XTMAP, namco_cus4xtmap_device)

#endif // MAME_NAMCO_NAMCO_CUS4XTMAP_H

