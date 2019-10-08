// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/*************************************************************************

    Mr. Jong

*************************************************************************/
#ifndef MAME_INCLUDES_MRJONG_H
#define MAME_INCLUDES_MRJONG_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class mrjong_state : public driver_device
{
public:
	mrjong_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void mrjong(machine_config &config);

private:
	DECLARE_READ8_MEMBER(io_0x03_r);
	DECLARE_WRITE8_MEMBER(mrjong_videoram_w);
	DECLARE_WRITE8_MEMBER(mrjong_colorram_w);
	DECLARE_WRITE8_MEMBER(mrjong_flipscreen_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	void mrjong_palette(palette_device &palette) const;
	uint32_t screen_update_mrjong(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );

	void mrjong_io_map(address_map &map);
	void mrjong_map(address_map &map);

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	/* video-related */
	tilemap_t *m_bg_tilemap = nullptr;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

#endif // MAME_INCLUDES_MRJONG_H
