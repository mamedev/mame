// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_NAMCO_NAMCO_C123TMAP_H
#define MAME_NAMCO_NAMCO_C123TMAP_H

#pragma once

#include "screen.h"
#include "tilemap.h"

class namco_c123tmap_device : public device_t, public device_gfx_interface
{
public:
	// construction/destruction
	namco_c123tmap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void set_color_base(int color) { m_color_base = color; }
	void set_offset(int xoffs, int yoffs)
	{
		m_xoffs = xoffs;
		m_yoffs = yoffs;
	}
	void set_tmap3_half_height() { m_tmap3_half_height = true; }

	typedef delegate<void(uint16_t, int*, int*)> c123_tilemap_delegate;
	void set_tile_callback(c123_tilemap_delegate tilemap_cb) { m_tilemapinfo.cb = tilemap_cb; }

	// 16 bit handlers
	void videoram16_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t videoram16_r(offs_t offset);
	void control16_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t control16_r(offs_t offset);

	// 8 bit handlers
	void videoram8_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	uint8_t videoram8_r(offs_t offset);
	void control8_w(offs_t offset, uint8_t data);
	uint8_t control8_r(offs_t offset);

	void mark_all_dirty(void);
	void init_scroll(int flip);
	void draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, int prival = 0, int primask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	template<int Offset> TILE_GET_INFO_MEMBER(get_tile_info);
	void set_tilemap_videoram(int offset, uint16_t newword);

	struct info
	{
		uint16_t control[0x40 / 2];
		/**
		 * [0x1] 0x02/2 tilemap#0.scrollx
		 * [0x3] 0x06/2 tilemap#0.scrolly
		 * [0x5] 0x0a/2 tilemap#1.scrollx
		 * [0x7] 0x0e/2 tilemap#1.scrolly
		 * [0x9] 0x12/2 tilemap#2.scrollx
		 * [0xb] 0x16/2 tilemap#2.scrolly
		 * [0xd] 0x1a/2 tilemap#3.scrollx
		 * [0xf] 0x1e/2 tilemap#3.scrolly
		 * 0x20/2 priority
		 * 0x30/2 color
		 */
		tilemap_t *tmap[6];
		std::unique_ptr<uint16_t[]> videoram;
		c123_tilemap_delegate cb;
	};

	info m_tilemapinfo;
	DECLARE_GFXDECODE_MEMBER(gfxinfo);

	int m_color_base;
	int m_xoffs, m_yoffs;
	bool m_tmap3_half_height;
	required_region_ptr<uint8_t> m_mask;
};

// device type definition
DECLARE_DEVICE_TYPE(NAMCO_C123TMAP, namco_c123tmap_device)

#endif // MAME_NAMCO_NAMCO_C123TMAP_H

