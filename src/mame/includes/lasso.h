// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Nicola Salmoria, Luca Elia
/***************************************************************************

 Lasso and similar hardware

***************************************************************************/
#ifndef MAME_INCLUDES_LASSO_H
#define MAME_INCLUDES_LASSO_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/sn76496.h"
#include "emupal.h"

class lasso_state : public driver_device
{
public:
	lasso_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_back_color(*this, "back_color"),
		m_chip_data(*this, "chip_data"),
		m_bitmap_ram(*this, "bitmap_ram"),
		m_last_colors(*this, "last_colors"),
		m_track_scroll(*this, "track_scroll"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_sn_1(*this, "sn76489.1"),
		m_sn_2(*this, "sn76489.2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void base(machine_config &config);
	void wwjgtin(machine_config &config);
	void lasso(machine_config &config);
	void chameleo(machine_config &config);
	void pinbo(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_back_color;
	optional_shared_ptr<uint8_t> m_chip_data;
	optional_shared_ptr<uint8_t> m_bitmap_ram;    /* 0x2000 bytes for a 256 x 256 x 1 bitmap */
	optional_shared_ptr<uint8_t> m_last_colors;
	optional_shared_ptr<uint8_t> m_track_scroll;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_track_tilemap;
	uint8_t    m_gfxbank;     /* used by lasso, chameleo, wwjgtin and pinbo */
	uint8_t    m_track_enable;    /* used by wwjgtin */

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<sn76489_device> m_sn_1;
	optional_device<sn76489_device> m_sn_2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_READ8_MEMBER(sound_status_r);
	DECLARE_WRITE8_MEMBER(sound_select_w);
	DECLARE_WRITE8_MEMBER(lasso_videoram_w);
	DECLARE_WRITE8_MEMBER(lasso_colorram_w);
	DECLARE_WRITE8_MEMBER(lasso_flip_screen_w);
	DECLARE_WRITE8_MEMBER(lasso_video_control_w);
	DECLARE_WRITE8_MEMBER(wwjgtin_video_control_w);
	DECLARE_WRITE8_MEMBER(pinbo_video_control_w);
	TILE_GET_INFO_MEMBER(lasso_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(wwjgtin_get_track_tile_info);
	TILE_GET_INFO_MEMBER(pinbo_get_bg_tile_info);
	void lasso_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(wwjgtin);
	DECLARE_MACHINE_RESET(wwjgtin);
	DECLARE_VIDEO_START(wwjgtin);
	void wwjgtin_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(pinbo);
	uint32_t screen_update_lasso(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_chameleo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_wwjgtin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	static rgb_t get_color(int data);
	void wwjgtin_set_last_four_colors();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int reverse);
	void draw_lasso(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void chameleo_audio_map(address_map &map);
	void chameleo_main_map(address_map &map);
	void lasso_audio_map(address_map &map);
	void lasso_coprocessor_map(address_map &map);
	void lasso_main_map(address_map &map);
	void pinbo_audio_io_map(address_map &map);
	void pinbo_audio_map(address_map &map);
	void pinbo_main_map(address_map &map);
	void wwjgtin_audio_map(address_map &map);
	void wwjgtin_main_map(address_map &map);
};

#endif // MAME_INCLUDES_LASSO_H
