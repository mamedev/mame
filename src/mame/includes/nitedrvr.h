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

/* Discrete Sound Input Nodes */
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
		m_videoram(*this, "videoram"),
		m_hvc(*this, "hvc"),
		m_maincpu(*this, "maincpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_led(*this, "led")
	{ }

	void nitedrvr(machine_config &config);

private:
	DECLARE_READ8_MEMBER(nitedrvr_steering_reset_r);
	DECLARE_WRITE8_MEMBER(nitedrvr_steering_reset_w);
	DECLARE_READ8_MEMBER(nitedrvr_in0_r);
	DECLARE_READ8_MEMBER(nitedrvr_in1_r);
	DECLARE_WRITE8_MEMBER(nitedrvr_out0_w);
	DECLARE_WRITE8_MEMBER(nitedrvr_out1_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_nitedrvr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(nitedrvr_crash_toggle_callback);
	void draw_box(bitmap_ind16 &bitmap, const rectangle &cliprect, int bx, int by, int ex, int ey);
	void draw_roadway(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_tiles(bitmap_ind16 &bitmap, const rectangle &cliprect);
	int nitedrvr_steering();
	void nitedrvr_map(address_map &map);

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_hvc;

	/* input */
	uint8_t m_gear;
	uint8_t m_track;
	int32_t m_steering_buf;
	int32_t m_steering_val;
	uint8_t m_crash_en;
	uint8_t m_crash_data;
	uint8_t m_crash_data_en;  // IC D8
	uint8_t m_ac_line;
	int32_t m_last_steering_val;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	output_finder<> m_led;
};

/*----------- defined in audio/nitedrvr.c -----------*/
DISCRETE_SOUND_EXTERN( nitedrvr_discrete );

#endif // MAME_INCLUDES_NITEDRVR_H
