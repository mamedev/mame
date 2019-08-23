// license:BSD-3-Clause
// copyright-holders:Richard Davies
/*************************************************************************

    Exed Exes

*************************************************************************/
#ifndef MAME_INCLUDES_EXEDEXES_H
#define MAME_INCLUDES_EXEDEXES_H

#pragma once

#include "machine/timer.h"
#include "video/bufsprite.h"
#include "emupal.h"
#include "tilemap.h"

class exedexes_state : public driver_device
{
public:
	exedexes_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_nbg_yscroll(*this, "nbg_yscroll"),
		m_nbg_xscroll(*this, "nbg_xscroll"),
		m_bg_scroll(*this, "bg_scroll"),
		m_tilerom(*this, "tilerom"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void exedexes(machine_config &config);

private:
	/* memory pointers */
	required_device<buffered_spriteram8_device> m_spriteram;
	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u8> m_colorram;
	required_shared_ptr<u8> m_nbg_yscroll;
	required_shared_ptr<u8> m_nbg_xscroll;
	required_shared_ptr<u8> m_bg_scroll;
	required_region_ptr<u8> m_tilerom;

	/* video-related */
	tilemap_t      *m_bg_tilemap;
	tilemap_t      *m_fg_tilemap;
	tilemap_t      *m_tx_tilemap;
	int            m_chon;
	int            m_objon;
	int            m_sc1on;
	int            m_sc2on;

	void videoram_w(offs_t offset, u8 data);
	void colorram_w(offs_t offset, u8 data);
	void c804_w(u8 data);
	void gfxctrl_w(u8 data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILEMAP_MAPPER_MEMBER(bg_tilemap_scan);
	TILEMAP_MAPPER_MEMBER(fg_tilemap_scan);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void exedexes_palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void exedexes_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_EXEDEXES_H
