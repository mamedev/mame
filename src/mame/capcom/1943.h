// license:BSD-3-Clause
// copyright-holders:Paul Leaman
/***************************************************************************

    1943

***************************************************************************/
#ifndef MAME_CAPCOM_1943_H
#define MAME_CAPCOM_1943_H

#pragma once

#include "cpu/mcs51/mcs51.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class _1943_state : public driver_device
{
public:
	_1943_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
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
	void _1943b(machine_config &config);

	void init_1943();

private:
	/* devices / memory pointers */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<i8751_device> m_mcu;
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
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_bg2_tilemap = nullptr;
	int     m_char_on = 0;
	int     m_obj_on = 0;
	int     m_bg1_on = 0;
	int     m_bg2_on = 0;

	/* protection */
	u8 m_cpu_to_mcu = 0; // ls374 at 5k
	u8 m_mcu_to_cpu = 0; // ls374 at 6k
	u8 m_audiocpu_to_mcu = 0; // ls374 at 5l
	u8 m_mcu_to_audiocpu = 0; // ls374 at 6l
	u8 m_mcu_p0 = 0;
	u8 m_mcu_p2 = 0;
	u8 m_mcu_p3 = 0;

	void mcu_p3_w(u8 data);

	void videoram_w(offs_t offset, u8 data);
	void colorram_w(offs_t offset, u8 data);
	void c804_w(u8 data);
	void d806_w(u8 data);

	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void _1943_palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void _1943_drawgfx(bitmap_ind16 &dest_bmp,const rectangle &clip,gfx_element *gfx,
							u32 code,u32 color,bool flipx,bool flipy,int offsx,int offsy,
							u8 transparent_color);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void c1943_map(address_map &map) ATTR_COLD;
	void c1943b_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

#endif // MAME_CAPCOM_1943_H
