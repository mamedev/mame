// license:BSD-3-Clause
// copyright-holders:Paul Leaman
/***************************************************************************

    1943

***************************************************************************/
#ifndef MAME_INCLUDES_1943_H
#define MAME_INCLUDES_1943_H

#pragma once

#include "emupal.h"
#include "screen.h"

class _1943_state : public driver_device
{
public:
	_1943_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_scrollx(*this, "scrollx"),
		m_scrolly(*this, "scrolly"),
		m_bgscrollx(*this, "bgscrollx"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_tilerom(*this, "tilerom"),
		m_proms(*this, "proms"),
		m_mainbank(*this, "mainbank")
	{ }

	void _1943(machine_config &config);

	void init_1943b();
	void init_1943();

private:
	/* devices / memory pointers */
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u8> m_colorram;
	required_shared_ptr<u8> m_scrollx;
	required_shared_ptr<u8> m_scrolly;
	required_shared_ptr<u8> m_bgscrollx;
	required_shared_ptr<u8> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_region_ptr<u8> m_tilerom;
	required_region_ptr<u8> m_proms;
	required_memory_bank m_mainbank;

	/* video-related */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_bg2_tilemap;
	int     m_char_on;
	int     m_obj_on;
	int     m_bg1_on;
	int     m_bg2_on;

	/* protection */
	u8   m_prot_value;
	void protection_w(u8 data);
	u8 protection_r();
	u8 _1943b_c007_r();

	void videoram_w(offs_t offset, u8 data);
	void colorram_w(offs_t offset, u8 data);
	void c804_w(u8 data);
	void d806_w(u8 data);

	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void _1943_palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void _1943_drawgfx(bitmap_ind16 &dest_bmp,const rectangle &clip,gfx_element *gfx,
							u32 code,u32 color,bool flipx,bool flipy,int offsx,int offsy,
							u8 transparent_color);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void c1943_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_1943_H
