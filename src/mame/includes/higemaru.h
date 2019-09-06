// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/*************************************************************************

    Pirate Ship Higemaru

*************************************************************************/
#ifndef MAME_INCLUDES_HIGEMARU_H
#define MAME_INCLUDES_HIGEMARU_H

#pragma once

#include "machine/timer.h"
#include "emupal.h"
#include "tilemap.h"

class higemaru_state : public driver_device
{
public:
	higemaru_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void higemaru(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(higemaru_videoram_w);
	DECLARE_WRITE8_MEMBER(higemaru_colorram_w);
	DECLARE_WRITE8_MEMBER(higemaru_c800_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	void higemaru_palette(palette_device &palette) const;
	uint32_t screen_update_higemaru(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(higemaru_scanline);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void higemaru_map(address_map &map);
};

#endif // MAME_INCLUDES_HIGEMARU_H
