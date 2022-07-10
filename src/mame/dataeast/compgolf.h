// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Pierpaolo Prazzoli, Bryan McPhail
/*************************************************************************

    Competition Golf Final Round

*************************************************************************/
#ifndef MAME_INCLUDES_COMPGOLF_H
#define MAME_INCLUDES_COMPGOLF_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

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

	void init_compgolf();
	void compgolf(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_bg_ram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t        *m_text_tilemap = nullptr;
	tilemap_t        *m_bg_tilemap = nullptr;
	int            m_scrollx_lo = 0;
	int            m_scrollx_hi = 0;
	int            m_scrolly_lo = 0;
	int            m_scrolly_hi = 0;

	/* misc */
	int            m_bank = 0;
	void compgolf_ctrl_w(uint8_t data);
	void compgolf_video_w(offs_t offset, uint8_t data);
	void compgolf_back_w(offs_t offset, uint8_t data);
	void compgolf_scrollx_lo_w(uint8_t data);
	void compgolf_scrolly_lo_w(uint8_t data);
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
	void compgolf_map(address_map &map);
};

#endif // MAME_INCLUDES_COMPGOLF_H
