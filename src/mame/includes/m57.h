// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
#ifndef MAME_INCLUDES_M57_H
#define MAME_INCLUDES_M57_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class m57_state : public driver_device
{
public:
	m57_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_scrollram(*this, "scrollram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void m57(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_scrollram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t*             m_bg_tilemap = nullptr;
	int                  m_flipscreen = 0;
	void m57_videoram_w(offs_t offset, uint8_t data);
	void m57_flipscreen_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void video_start() override;
	void m57_palette(palette_device &palette) const;
	uint32_t screen_update_m57(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void main_map(address_map &map);
};

#endif // MAME_INCLUDES_M57_H
