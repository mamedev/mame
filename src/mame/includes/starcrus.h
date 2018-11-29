// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo
#ifndef MAME_INCLUDES_STARCRUS_H
#define MAME_INCLUDES_STARCRUS_H

#pragma once

#include "sound/samples.h"
#include "emupal.h"

class starcrus_state : public driver_device
{
public:
	starcrus_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_led(*this, "led2")
	{ }

	void starcrus(machine_config &config);

private:
	DECLARE_WRITE8_MEMBER(s1_x_w);
	DECLARE_WRITE8_MEMBER(s1_y_w);
	DECLARE_WRITE8_MEMBER(s2_x_w);
	DECLARE_WRITE8_MEMBER(s2_y_w);
	DECLARE_WRITE8_MEMBER(p1_x_w);
	DECLARE_WRITE8_MEMBER(p1_y_w);
	DECLARE_WRITE8_MEMBER(p2_x_w);
	DECLARE_WRITE8_MEMBER(p2_y_w);
	DECLARE_WRITE8_MEMBER(ship_parm_1_w);
	DECLARE_WRITE8_MEMBER(ship_parm_2_w);
	DECLARE_WRITE8_MEMBER(proj_parm_1_w);
	DECLARE_WRITE8_MEMBER(proj_parm_2_w);
	DECLARE_READ8_MEMBER(coll_det_r);

	virtual void machine_start() override;
	virtual void video_start() override;

	required_device<cpu_device> m_maincpu;
	required_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	output_finder<> m_led;

	std::unique_ptr<bitmap_ind16> m_ship1_vid;
	std::unique_ptr<bitmap_ind16> m_ship2_vid;
	std::unique_ptr<bitmap_ind16> m_proj1_vid;
	std::unique_ptr<bitmap_ind16> m_proj2_vid;

	int m_s1_x;
	int m_s1_y;
	int m_s2_x;
	int m_s2_y;
	int m_p1_x;
	int m_p1_y;
	int m_p2_x;
	int m_p2_y;

	int m_p1_sprite;
	int m_p2_sprite;
	int m_s1_sprite;
	int m_s2_sprite;

	int m_engine1_on;
	int m_engine2_on;
	int m_explode1_on;
	int m_explode2_on;
	int m_launch1_on;
	int m_launch2_on;

	int m_collision_reg;

	int m_engine_sound_playing;
	int m_explode_sound_playing;
	int m_launch1_sound_playing;
	int m_launch2_sound_playing;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int collision_check_s1s2();
	int collision_check_p1p2();
	int collision_check_s1p1p2();
	int collision_check_s2p1p2();

	void starcrus_io_map(address_map &map);
	void starcrus_map(address_map &map);
};

#endif // MAME_INCLUDES_STARCRUS_H
