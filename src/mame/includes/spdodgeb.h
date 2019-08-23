// license:BSD-3-Clause
// copyright-holders:Paul Hampson, Nicola Salmoria
/*************************************************************************

    Super Dodge Ball hardware

*************************************************************************/
#ifndef MAME_INCLUDES_SPDODGEB_H
#define MAME_INCLUDES_SPDODGEB_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/msm5205.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class spdodgeb_state : public driver_device
{
public:
	spdodgeb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm1(*this, "msm1"),
		m_msm2(*this, "msm2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram")
	{ }

	void spdodgeb(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(mcu63705_busy_r);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm1;
	required_device<msm5205_device> m_msm2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	int m_toggle;
	int m_adpcm_pos[2];
	int m_adpcm_end[2];
	int m_adpcm_idle[2];
	int m_adpcm_data[2];
	int m_mcu63701_command;
	int m_inputs[4];
	uint8_t m_tapc[4];
	uint8_t m_last_port[2];
	uint8_t m_last_dash[2];
#if 0
	int m_running[2];
	int m_jumped[2];
	int m_prev[2][2];
	int m_countup[2][2];
	int m_countdown[2][2];
	int m_prev[2];
#endif
	int m_tile_palbank;
	int m_sprite_palbank;
	tilemap_t *m_bg_tilemap;
	int m_lastscroll;

	DECLARE_WRITE8_MEMBER(spd_adpcm_w);
	DECLARE_READ8_MEMBER(mcu63701_r);
	DECLARE_WRITE8_MEMBER(mcu63701_w);
	DECLARE_WRITE8_MEMBER(scrollx_lo_w);
	DECLARE_WRITE8_MEMBER(ctrl_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE_LINE_MEMBER(spd_adpcm_int_1);
	DECLARE_WRITE_LINE_MEMBER(spd_adpcm_int_2);

	TILEMAP_MAPPER_MEMBER(background_scan);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void spdodgeb_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	void mcu63705_update_inputs();
	void spd_adpcm_int(msm5205_device *device, int chip);
	void spdodgeb_map(address_map &map);
	void spdodgeb_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_SPDODGEB_H
