// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/*************************************************************************

    Atari Skydiver hardware

*************************************************************************/
#ifndef MAME_INCLUDES_SKYDIVER_H
#define MAME_INCLUDES_SKYDIVER_H

#pragma once

#include "machine/74259.h"
#include "machine/watchdog.h"
#include "sound/discrete.h"
#include "emupal.h"
#include "tilemap.h"

/* Discrete Sound Input Nodes */
#define SKYDIVER_RANGE_DATA     NODE_01
#define SKYDIVER_NOTE_DATA      NODE_02
#define SKYDIVER_RANGE3_EN      NODE_03
#define SKYDIVER_NOISE_DATA     NODE_04
#define SKYDIVER_NOISE_RST      NODE_05
#define SKYDIVER_WHISTLE1_EN    NODE_06
#define SKYDIVER_WHISTLE2_EN    NODE_07
#define SKYDIVER_OCT1_EN        NODE_08
#define SKYDIVER_OCT2_EN        NODE_09
#define SKYDIVER_SOUND_EN       NODE_10


class skydiver_state : public driver_device
{
public:
	skydiver_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_latch3(*this, "latch3"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_leds(*this, "led%u", 0U),
		m_lamp_s(*this, "lamps"),
		m_lamp_k(*this, "lampk"),
		m_lamp_y(*this, "lampy"),
		m_lamp_d(*this, "lampd"),
		m_lamp_i(*this, "lampi"),
		m_lamp_v(*this, "lampv"),
		m_lamp_e(*this, "lampe"),
		m_lamp_r(*this, "lampr")
	{ }

	void skydiver(machine_config &config);

private:
	DECLARE_WRITE_LINE_MEMBER(nmion_w);
	void videoram_w(offs_t offset, uint8_t data);
	uint8_t wram_r(offs_t offset);
	void wram_w(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(width_w);
	DECLARE_WRITE_LINE_MEMBER(coin_lockout_w);
	DECLARE_WRITE_LINE_MEMBER(start_lamp_1_w);
	DECLARE_WRITE_LINE_MEMBER(start_lamp_2_w);
	DECLARE_WRITE_LINE_MEMBER(lamp_s_w);
	DECLARE_WRITE_LINE_MEMBER(lamp_k_w);
	DECLARE_WRITE_LINE_MEMBER(lamp_y_w);
	DECLARE_WRITE_LINE_MEMBER(lamp_d_w);
	DECLARE_WRITE_LINE_MEMBER(lamp_i_w);
	DECLARE_WRITE_LINE_MEMBER(lamp_v_w);
	DECLARE_WRITE_LINE_MEMBER(lamp_e_w);
	DECLARE_WRITE_LINE_MEMBER(lamp_r_w);
	void latch3_watchdog_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_tile_info);

	virtual void machine_reset() override;
	virtual void video_start() override;
	void skydiver_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(interrupt);
	void skydiver_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<f9334_device> m_latch3;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;

	output_finder<2> m_leds;
	output_finder<> m_lamp_s;
	output_finder<> m_lamp_k;
	output_finder<> m_lamp_y;
	output_finder<> m_lamp_d;
	output_finder<> m_lamp_i;
	output_finder<> m_lamp_v;
	output_finder<> m_lamp_e;
	output_finder<> m_lamp_r;
	int m_nmion = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	int m_width = 0;
};

/*----------- defined in audio/skydiver.c -----------*/
DISCRETE_SOUND_EXTERN( skydiver_discrete );

#endif // MAME_INCLUDES_SKYDIVER_H
