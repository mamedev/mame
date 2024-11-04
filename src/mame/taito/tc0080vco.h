// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_TAITO_TC0080VCO_H
#define MAME_TAITO_TC0080VCO_H

#pragma once

#include "tilemap.h"


class tc0080vco_device : public device_t, public device_gfx_interface
{
public:
	tc0080vco_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	void set_colbase(u16 base) { m_colbase = base; }
	void set_offsets(int x_offset, int y_offset)
	{
		m_bg_xoffs = x_offset;
		m_bg_yoffs = y_offset;
	}
	void set_bgflip_yoffs(int offs) { m_bg_flip_yoffs = offs; }

	u16 word_r(offs_t offset);
	void word_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void get_sprite_params(int offs, bool zoomy_enable);
	void draw_single_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void tilemap_update();
	void tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, u8 priority, u8 pmask = 0xff);
	void set_fg0_debug(bool debug) { m_has_fg0 = debug ? 0 : 1; }

	u16 cram_0_r(int offset);
	u16 cram_1_r(int offset);
	u16 sprram_r(int offset);
	u16 scrram_r(int offset);
	void scrollram_w(offs_t offset, u16 data);
	int flipscreen_r();

	int get_sprite_x() { return m_sprite.x0; }
	int get_sprite_y() { return m_sprite.y0; }
	u32 get_sprite_tile_offs() { return m_sprite.tile_offs; }
	int get_sprite_zoomx() { return m_sprite.zoomx; }
	int get_sprite_zoomy() { return m_sprite.zoomy; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	// internal state
	struct sprite_t
	{
		int x0 = 0, y0 = 0;
		int zoomx = 0, zoomy = 0;
		u32 tile_offs = 0;
		u8 ysize = 0;
		int dx = 0, ex = 0, zx = 0;
		int dy = 0, ey = 0, zy = 0;
	};

	std::unique_ptr<u16[]>       m_ram;
	u16 *       m_bg0_ram_0;
	u16 *       m_bg0_ram_1;
	u16 *       m_bg1_ram_0;
	u16 *       m_bg1_ram_1;
	u16 *       m_tx_ram_0;
	u16 *       m_tx_ram_1;
	u16 *       m_char_ram;
	u16 *       m_bgscroll_ram;

/* FIXME: This sprite related stuff still needs to be accessed in video/taito_h */
	u16 *       m_chain_ram_0;
	u16 *       m_chain_ram_1;
	u16 *       m_spriteram;
	u16 *       m_scroll_ram;

	u16         m_bg0_scrollx;
	u16         m_bg0_scrolly;
	u16         m_bg1_scrollx;
	u16         m_bg1_scrolly;

	tilemap_t   *m_tilemap[3];

	bool        m_flipscreen;

	int         m_bg_xoffs, m_bg_yoffs;
	int         m_bg_flip_yoffs;
	int         m_has_fg0; // for debug, it can be enabled with set_fg0_debug(true)
	sprite_t    m_sprite;
	u16         m_colbase;

	required_memory_region m_gfx_region;

	TILE_GET_INFO_MEMBER(get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	void bg0_tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, u8 priority, u8 pmask = 0xff);
	void bg1_tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, u8 priority, u8 pmask = 0xff);
};

DECLARE_DEVICE_TYPE(TC0080VCO, tc0080vco_device)

#endif // MAME_TAITO_TC0080VCO_H
