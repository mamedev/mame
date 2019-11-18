// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************

    American Speedway

*************************************************************************/
#ifndef MAME_INCLUDES_AMSPDWY_H
#define MAME_INCLUDES_AMSPDWY_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/ym2151.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class amspdwy_state : public driver_device
{
public:
	amspdwy_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_ym2151(*this, "ymsnd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void amspdwy(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_colorram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<ym2151_device> m_ym2151;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	int m_flipscreen;

	/* misc */
	uint8_t m_wheel_old[2];
	uint8_t m_wheel_return[2];

	DECLARE_READ8_MEMBER(amspdwy_wheel_0_r);
	DECLARE_READ8_MEMBER(amspdwy_wheel_1_r);
	DECLARE_WRITE8_MEMBER(amspdwy_flipscreen_w);
	DECLARE_WRITE8_MEMBER(amspdwy_videoram_w);
	DECLARE_WRITE8_MEMBER(amspdwy_colorram_w);
	DECLARE_READ8_MEMBER(amspdwy_sound_r);
	TILE_GET_INFO_MEMBER(get_tile_info);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_cols_back);

	uint32_t screen_update_amspdwy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint8_t amspdwy_wheel_r(int index);

	void amspdwy_map(address_map &map);
	void amspdwy_portmap(address_map &map);
	void amspdwy_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_AMSPDWY_H
