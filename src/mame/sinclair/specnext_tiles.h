// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_SINCLAIR_SPECNEXT_TILES_H
#define MAME_SINCLAIR_SPECNEXT_TILES_H

#pragma once

#include "tilemap.h"

class specnext_tiles_device : public device_t, public device_gfx_interface
{

public:
	specnext_tiles_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	specnext_tiles_device &set_raster_offset(u16 offset_h,  u16 offset_v) { m_offset_h = offset_h - (OVER_BORDER << 1); m_offset_v = offset_v - OVER_BORDER; return *this; }
	specnext_tiles_device &set_host_ram_ptr(const u8 *host_ram_ptr) { m_host_ram_ptr = host_ram_ptr; return *this; }
	specnext_tiles_device &set_palette(const char *tag, u16 base_offset, u16 alt_offset);
	void tilemap_update();

	void control_w(u8 control) { m_control = control; tilemap_update(); }
	void default_flags_w(u8 default_flags) { m_default_flags = default_flags; m_tilemap[0]->mark_all_dirty(); m_tilemap[1]->mark_all_dirty(); }
	void transp_colour_w(u8 transp_colour) { m_transp_colour = transp_colour; tilemap_update(); }

	void tm_map_base_w(u8 tm_map_base) { m_tm_map_base = tm_map_base; m_tilemap[0]->mark_all_dirty(); m_tilemap[1]->mark_all_dirty(); }
	void tm_tile_base_w(u8 tm_tile_base) { m_tm_tile_base = tm_tile_base; tilemap_update(); }

	void tm_scroll_x_w(u16 tm_scroll_x) { m_tm_scroll_x = tm_scroll_x; tilemap_update(); }
	void tm_scroll_y_w(u8 tm_scroll_y) { m_tm_scroll_y = tm_scroll_y; tilemap_update(); }

	void clip_x1_w(u8 clip_x1) { m_clip_x1 = clip_x1; }
	void clip_x2_w(u8 clip_x2) { m_clip_x2 = clip_x2; }
	void clip_y1_w(u8 clip_y1) { m_clip_y1 = clip_y1; }
	void clip_y2_w(u8 clip_y2) { m_clip_y2 = clip_y2; }

	void draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 flags, u8 pcode = 0xff);

protected:
	static constexpr u8 OVER_BORDER = 32;
	static constexpr rectangle SCREEN_AREA = { 0, 319, 0, 255 };

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	TILE_GET_INFO_MEMBER(get_tile_info);

	const u8 *m_host_ram_ptr;
	const u8 *m_tiles_info;
	tilemap_t *m_tilemap[2];

private:
	u16 m_palette_base_offset;
	u16 m_palette_alt_offset;
	bool m_tm_palette_select;

	u16 m_offset_h, m_offset_v;

	u8 m_control; // u7
	u8 m_default_flags;
	u8 m_transp_colour;

	u8 m_tm_map_base;
	u8 m_tm_tile_base;

	u16 m_tm_scroll_x;
	u8 m_tm_scroll_y;

	u8 m_clip_x1;
	u8 m_clip_x2;
	u8 m_clip_y1;
	u8 m_clip_y2;
};


DECLARE_DEVICE_TYPE(SPECNEXT_TILES, specnext_tiles_device)
#endif // MAME_SINCLAIR_SPECNEXT_TILES_H
