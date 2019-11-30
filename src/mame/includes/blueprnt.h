// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Blue Print

***************************************************************************/
#ifndef MAME_INCLUDES_BLUEPRNT_H
#define MAME_INCLUDES_BLUEPRNT_H

#pragma once

#include "machine/gen_latch.h"
#include "emupal.h"
#include "tilemap.h"

class blueprnt_state : public driver_device
{
public:
	blueprnt_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_scrollram(*this, "scrollram"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram")
	{ }

	/* device/memory pointers */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_scrollram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_colorram;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	int     m_gfx_bank;

	/* misc */
	int     m_dipsw;

	DECLARE_READ8_MEMBER(blueprnt_sh_dipsw_r);
	DECLARE_READ8_MEMBER(grasspin_sh_dipsw_r);
	DECLARE_WRITE8_MEMBER(blueprnt_sound_command_w);
	DECLARE_WRITE8_MEMBER(blueprnt_coin_counter_w);
	DECLARE_WRITE8_MEMBER(blueprnt_videoram_w);
	DECLARE_WRITE8_MEMBER(blueprnt_colorram_w);
	DECLARE_WRITE8_MEMBER(blueprnt_flipscreen_w);
	DECLARE_WRITE8_MEMBER(dipsw_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_VIDEO_START(blueprnt);
	void blueprnt_palette(palette_device &palette) const;
	uint32_t screen_update_blueprnt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void blueprnt(machine_config &config);
	void grasspin(machine_config &config);
	void blueprnt_map(address_map &map);
	void grasspin_map(address_map &map);
	void sound_io(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_BLUEPRNT_H
