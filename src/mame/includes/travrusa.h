// license:BSD-3-Clause
// copyright-holders:Lee Taylor
// thanks-to:John Clegg,Tomasz Slanina
#ifndef MAME_INCLUDES_TRAVRUSA_H
#define MAME_INCLUDES_TRAVRUSA_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class travrusa_state : public driver_device
{
public:
	travrusa_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void shtrider(machine_config &config);
	void travrusa(machine_config &config);
	void shtriderb(machine_config &config);

	void init_shtridra();
	void init_motorace();
	void init_shtridrb();

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t*             m_bg_tilemap;
	int                  m_scrollx[2];
	DECLARE_WRITE8_MEMBER(travrusa_videoram_w);
	DECLARE_WRITE8_MEMBER(travrusa_scroll_x_low_w);
	DECLARE_WRITE8_MEMBER(travrusa_scroll_x_high_w);
	DECLARE_WRITE8_MEMBER(travrusa_flipscreen_w);
	DECLARE_READ8_MEMBER(shtridrb_port11_r);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_reset() override;
	virtual void video_start() override;
	void travrusa_palette(palette_device &palette) const;
	void shtrider_palette(palette_device &palette) const;
	uint32_t screen_update_travrusa(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void set_scroll();
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void main_map(address_map &map);
};

#endif // MAME_INCLUDES_TRAVRUSA_H
