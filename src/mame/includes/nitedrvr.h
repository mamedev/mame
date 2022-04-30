// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/*************************************************************************

    Atari Night Driver hardware

*************************************************************************/
#ifndef MAME_INCLUDES_NITEDRVR_H
#define MAME_INCLUDES_NITEDRVR_H

#pragma once

#include "machine/timer.h"
#include "sound/discrete.h"
#include "emupal.h"

// Discrete Sound Input Nodes
#define NITEDRVR_BANG_DATA  NODE_01
#define NITEDRVR_SKID1_EN   NODE_02
#define NITEDRVR_SKID2_EN   NODE_03
#define NITEDRVR_MOTOR_DATA NODE_04
#define NITEDRVR_CRASH_EN   NODE_05
#define NITEDRVR_ATTRACT_EN NODE_06


class nitedrvr_state : public driver_device
{
public:
	nitedrvr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_hvc(*this, "hvc"),
		m_steer(*this, "STEER"),
		m_gears(*this, "GEARS"),
		m_in0(*this, "IN0"),
		m_dsw(*this, "DSW%u", 0U),
		m_led(*this, "led"),
		m_track_sel(*this, "track%u", 1U),
		m_gear_sel(*this, "gear%u", 1U)
	{ }

	void nitedrvr(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	uint8_t steering_reset_r();
	void steering_reset_w(uint8_t data);
	uint8_t in0_r(offs_t offset);
	uint8_t in1_r(offs_t offset);
	void out0_w(uint8_t data);
	void out1_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(crash_toggle_callback);
	void draw_box(bitmap_ind16 &bitmap, const rectangle &cliprect, int bx, int by, int ex, int ey);
	void draw_roadway(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_tiles(bitmap_ind16 &bitmap, const rectangle &cliprect);
	int steering();
	void main_map(address_map &map);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// memory pointers
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_hvc;

	// input
	uint8_t m_gear = 0;
	uint8_t m_track = 0;
	int32_t m_steering_buf = 0;
	int32_t m_steering_val = 0;
	uint8_t m_crash_en = 0;
	uint8_t m_crash_data = 0;
	uint8_t m_crash_data_en = 0;  // IC D8
	uint8_t m_ac_line = 0;
	int32_t m_last_steering_val = 0;
	required_ioport m_steer, m_gears, m_in0;
	required_ioport_array<3> m_dsw;

	// output
	output_finder<> m_led;
	output_finder<3> m_track_sel;
	output_finder<4> m_gear_sel;
};

//----------- defined in audio/nitedrvr.cpp -----------
DISCRETE_SOUND_EXTERN( nitedrvr_discrete );

#endif // MAME_INCLUDES_NITEDRVR_H
