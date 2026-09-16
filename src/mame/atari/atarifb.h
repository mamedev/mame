// license:BSD-3-Clause
// copyright-holders:Mike Balfour, Patrick Lawrence, Brad Oliver
/*************************************************************************

    Atari Football hardware

*************************************************************************/

#ifndef MAME_ATARI_ATARIFB_H
#define MAME_ATARI_ATARIFB_H

#pragma once

#include "machine/timer.h"
#include "sound/discrete.h"
#include "video/pwm.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


/* Discrete Sound Input Nodes */
#define ATARIFB_WHISTLE_EN      NODE_01
#define ATARIFB_CROWD_DATA      NODE_02
#define ATARIFB_ATTRACT_EN      NODE_03
#define ATARIFB_NOISE_EN        NODE_04
#define ATARIFB_HIT_EN          NODE_05


class atarifb_state : public driver_device
{
public:
	atarifb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_led_pwm(*this, "led_pwm"),
		m_leds(*this, "led%u", 0U),
		m_alphap1_videoram(*this, "p1_videoram"),
		m_alphap2_videoram(*this, "p2_videoram"),
		m_field_videoram(*this, "field_videoram"),
		m_spriteram(*this, "spriteram"),
		m_scroll_register(*this, "scroll_register")
	{ }

	void atarifb4(machine_config &config);
	void atarifb(machine_config &config);
	void soccer(machine_config &config);
	void abaseb(machine_config &config);

private:
	void intack_w(uint8_t data);
	void atarifb_out1_w(uint8_t data);
	void atarifb4_out1_w(uint8_t data);
	void abaseb_out1_w(uint8_t data);
	void soccer_out1_w(uint8_t data);
	void atarifb_out2_w(uint8_t data);
	void soccer_out2_w(uint8_t data);
	void atarifb_out3_w(uint8_t data);
	uint8_t atarifb_in0_r();
	uint8_t atarifb_in2_r();
	uint8_t atarifb4_in0_r();
	uint8_t atarifb4_in2_r();
	void atarifb_alpha1_videoram_w(offs_t offset, uint8_t data);
	void atarifb_alpha2_videoram_w(offs_t offset, uint8_t data);
	void atarifb_field_videoram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(alpha1_get_tile_info);
	TILE_GET_INFO_MEMBER(alpha2_get_tile_info);
	TILE_GET_INFO_MEMBER(field_get_tile_info);
	void atarifb_palette(palette_device &palette) const;
	uint32_t screen_update_atarifb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_abaseb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_soccer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void get_tile_info_common( tile_data &tileinfo, tilemap_memory_index tile_index, uint8_t *alpha_videoram );
	void draw_playfield_and_alpha( bitmap_ind16 &bitmap, const rectangle &cliprect, int playfield_x_offset, int playfield_y_offset );
	void draw_sprites_atarifb( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_sprites_soccer( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void abaseb_map(address_map &map) ATTR_COLD;
	void atarifb4_map(address_map &map) ATTR_COLD;
	void atarifb_map(address_map &map) ATTR_COLD;
	void soccer_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<pwm_display_device> m_led_pwm;

	output_finder<2> m_leds;

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	/* video-related */
	required_shared_ptr<uint8_t> m_alphap1_videoram;
	required_shared_ptr<uint8_t> m_alphap2_videoram;
	required_shared_ptr<uint8_t> m_field_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scroll_register;

	tilemap_t  *m_alpha1_tilemap = nullptr;
	tilemap_t  *m_alpha2_tilemap = nullptr;
	tilemap_t  *m_field_tilemap = nullptr;

	/* sound-related */
	int m_ctrld = 0;
	int m_sign_x_1 = 0;
	int m_sign_y_1 = 0;
	int m_sign_x_2 = 0;
	int m_sign_y_2 = 0;
	int m_sign_x_3 = 0;
	int m_sign_y_3 = 0;
	int m_sign_x_4 = 0;
	int m_sign_y_4 = 0;
	int m_counter_x_in0 = 0;
	int m_counter_y_in0 = 0;
	int m_counter_x_in0b = 0;
	int m_counter_y_in0b = 0;
	int m_counter_x_in2 = 0;
	int m_counter_y_in2 = 0;
	int m_counter_x_in2b = 0;
	int m_counter_y_in2b = 0;
};

//----------- defined in audio/atarifb.cpp -----------
DISCRETE_SOUND_EXTERN( atarifb_discrete );
DISCRETE_SOUND_EXTERN( abaseb_discrete );

#endif // MAME_ATARI_ATARIFB_H
