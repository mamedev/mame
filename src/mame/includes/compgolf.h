// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Pierpaolo Prazzoli, Bryan McPhail
/*************************************************************************

    Competition Golf Final Round

*************************************************************************/
#ifndef MAME_INCLUDES_COMPGOLF_H
#define MAME_INCLUDES_COMPGOLF_H

#pragma once

#include "emupal.h"

class compgolf_state : public driver_device
{
public:
	compgolf_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_bg_ram(*this, "bg_ram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_bg_ram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t        *m_text_tilemap;
	tilemap_t        *m_bg_tilemap;
	int            m_scrollx_lo;
	int            m_scrollx_hi;
	int            m_scrolly_lo;
	int            m_scrolly_hi;

	/* misc */
	int            m_bank;
	DECLARE_WRITE8_MEMBER(compgolf_ctrl_w);
	DECLARE_WRITE8_MEMBER(compgolf_video_w);
	DECLARE_WRITE8_MEMBER(compgolf_back_w);
	DECLARE_WRITE8_MEMBER(compgolf_scrollx_lo_w);
	DECLARE_WRITE8_MEMBER(compgolf_scrolly_lo_w);
	void init_compgolf();
	TILE_GET_INFO_MEMBER(get_text_info);
	TILEMAP_MAPPER_MEMBER(back_scan);
	TILE_GET_INFO_MEMBER(get_back_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void compgolf_palette(palette_device &palette) const;
	uint32_t screen_update_compgolf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void compgolf_expand_bg();
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void compgolf(machine_config &config);
	void compgolf_map(address_map &map);
};

#endif // MAME_INCLUDES_COMPGOLF_H
