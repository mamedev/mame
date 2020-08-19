// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/*************************************************************************

    Atari Night Driver hardware

*************************************************************************/
#ifndef MAME_INCLUDES_NITEDRVR_H
#define MAME_INCLUDES_NITEDRVR_H

#pragma once

#include "machine/netlist.h"
#include "machine/timer.h"
#include "emupal.h"

class nitedrvr_state : public driver_device
{
public:
	nitedrvr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_hvc(*this, "hvc"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_speed1(*this, "sound_nl:speed1"),
		m_speed2(*this, "sound_nl:speed2"),
		m_speed3(*this, "sound_nl:speed3"),
		m_speed4(*this, "sound_nl:speed4"),
		m_skid1(*this, "sound_nl:skid1"),
		m_skid2(*this, "sound_nl:skid2"),
		m_crash(*this, "sound_nl:crash"),
		m_attract(*this, "sound_nl:attract"),
		m_led(*this, "led")
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
	void prg_map(address_map &map);

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
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<netlist_mame_logic_input_device> m_speed1;
	required_device<netlist_mame_logic_input_device> m_speed2;
	required_device<netlist_mame_logic_input_device> m_speed3;
	required_device<netlist_mame_logic_input_device> m_speed4;
	required_device<netlist_mame_logic_input_device> m_skid1;
	required_device<netlist_mame_logic_input_device> m_skid2;
	required_device<netlist_mame_logic_input_device> m_crash;
	required_device<netlist_mame_logic_input_device> m_attract;
	output_finder<> m_led;
};

#endif // MAME_INCLUDES_NITEDRVR_H
