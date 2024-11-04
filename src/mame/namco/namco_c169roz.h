
// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_NAMCO_NAMCO_C169ROZ_H
#define MAME_NAMCO_NAMCO_C169ROZ_H

#pragma once

#include "tilemap.h"

class namco_c169roz_device : public device_t, public device_gfx_interface
{
public:
	// construction/destruction
	namco_c169roz_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void set_is_namcofl(bool state) { m_is_namcofl = state; }
	void set_ram_words(uint32_t size) { m_ramsize = size; }
	void set_color_base(int color) { m_color_base = color; }

	uint16_t control_r(offs_t offset);
	void control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t videoram_r(offs_t offset);
	void videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	typedef delegate<void (uint16_t, int*, int*, int)> c169_tilemap_delegate;
	void set_tile_callback(c169_tilemap_delegate tilemap_cb) { m_c169_cb = tilemap_cb; }

	void draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri);
	void mark_all_dirty();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:

	c169_tilemap_delegate m_c169_cb;
	struct roz_parameters
	{
		uint32_t left, top, size;
		uint32_t startx, starty;
		int incxx, incxy, incyx, incyy;
		int color, priority;
		int wrap;
	};
	void unpack_params(const uint16_t *source, roz_parameters &params);
	void draw_helper(screen_device &screen, bitmap_ind16 &bitmap, tilemap_t &tmap, const rectangle &clip, const roz_parameters &params);
	void draw_scanline(screen_device &screen, bitmap_ind16 &bitmap, int line, int which, int pri, const rectangle &cliprect);
	void get_info(tile_data &tileinfo, int tile_index, int which);
	template<int Which> TILE_GET_INFO_MEMBER( get_info );
	TILEMAP_MAPPER_MEMBER( mapper );

	static const int ROZ_TILEMAP_COUNT = 2;
	tilemap_t *m_tilemap[ROZ_TILEMAP_COUNT]{};
	uint16_t m_control[0x20/2];
	std::vector<uint16_t> m_videoram;
	int m_color_base;
	DECLARE_GFXDECODE_MEMBER(gfxinfo);
	uint32_t m_ramsize = 0;

	// per-game hacks
	bool m_is_namcofl;

	required_region_ptr<uint8_t> m_mask;
};

// device type definition
DECLARE_DEVICE_TYPE(NAMCO_C169ROZ, namco_c169roz_device)

#endif // MAME_NAMCO_NAMCO_C169ROZ_H

