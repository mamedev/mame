// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Nicola Salmoria
/*
    buggychl
*/
#ifndef MAME_INCLUDES_BUGGYCHL_H
#define MAME_INCLUDES_BUGGYCHL_H

#pragma once

#include "machine/taito68705interface.h"
#include "machine/input_merger.h"
#include "machine/gen_latch.h"
#include "sound/msm5232.h"
#include "sound/ta7630.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"

class buggychl_state : public driver_device
{
public:
	buggychl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_charram(*this, "charram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_scrollv(*this, "scrollv"),
		m_scrollh(*this, "scrollh"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_bmcu(*this, "bmcu"),
		m_ta7630(*this, "ta7630"),
		m_msm(*this, "msm"),
		m_ay1(*this, "ay1"),
		m_ay2(*this, "ay2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundnmi(*this, "soundnmi"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_pedal_input(*this, "PEDAL"),
		m_led(*this, "led%u", 0U)
	{ }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_charram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scrollv;
	required_shared_ptr<uint8_t> m_scrollh;

	/* devices */
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_maincpu;
	required_device<taito68705_mcu_device> m_bmcu;
	required_device<ta7630_device> m_ta7630;
	required_device<msm5232_device> m_msm;
	required_device<ay8910_device> m_ay1;
	required_device<ay8910_device> m_ay2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<input_merger_device> m_soundnmi;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;
	required_ioport m_pedal_input;

	output_finder<1> m_led;

	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_WRITE8_MEMBER(sound_enable_w);
	DECLARE_READ8_MEMBER(mcu_status_r);
	DECLARE_READ8_MEMBER(sound_status_main_r);
	DECLARE_READ8_MEMBER(sound_status_sound_r);
	DECLARE_WRITE8_MEMBER(buggychl_chargen_w);
	DECLARE_WRITE8_MEMBER(buggychl_sprite_lookup_bank_w);
	DECLARE_WRITE8_MEMBER(buggychl_sprite_lookup_w);
	DECLARE_WRITE8_MEMBER(buggychl_ctrl_w);
	DECLARE_WRITE8_MEMBER(buggychl_bg_scrollx_w);
	DECLARE_WRITE8_MEMBER(ta7630_volbal_ay1_w);
	DECLARE_WRITE8_MEMBER(port_b_0_w);
	DECLARE_WRITE8_MEMBER(ta7630_volbal_ay2_w);
	DECLARE_WRITE8_MEMBER(port_b_1_w);
	DECLARE_WRITE8_MEMBER(ta7630_volbal_msm_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void buggychl_palette(palette_device &palette) const;
	uint32_t screen_update_buggychl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_CUSTOM_INPUT_MEMBER( pedal_in_r );

	void buggychl(machine_config &config);
	void buggychl_map(address_map &map);
	void sound_map(address_map &map);
private:
	void draw_sky( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_bg( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_fg( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	/* video-related */
	bitmap_ind16 m_tmp_bitmap1;
	bitmap_ind16 m_tmp_bitmap2;
	int         m_sl_bank;
	int         m_bg_clip_on;
	int         m_sky_on;
	int         m_sprite_color_base;
	int         m_bg_scrollx;
	bool        m_sound_irq_enable;
	uint8_t       m_sprite_lookup[0x2000];
};

#endif // MAME_INCLUDES_BUGGYCHL_H
