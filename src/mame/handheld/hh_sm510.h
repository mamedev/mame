// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM5xx family handhelds.

*/

#ifndef MAME_HANDHELD_HH_SM510_H
#define MAME_HANDHELD_HH_SM510_H

#pragma once

#include "cpu/sm510/sm510.h"
#include "cpu/sm510/sm511.h"
#include "cpu/sm510/sm530.h"
#include "cpu/sm510/sm5a.h"
#include "sound/spkrdev.h"


#define PORT_CHANGED_CB(x) \
	PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, x, 0)


class hh_sm510_state : public driver_device
{
public:
	hh_sm510_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker"),
		m_inputs(*this, "IN.%u", 0),
		m_io_ba(*this, "BA"),
		m_io_b(*this, "B"),
		m_out_x(*this, "%u.%u.%u", 0U, 0U, 0U)
	{ }

	virtual DECLARE_INPUT_CHANGED_MEMBER(input_changed);
	virtual DECLARE_INPUT_CHANGED_MEMBER(acl_button);

protected:
	// devices
	required_device<sm510_base_device> m_maincpu;
	optional_device<speaker_sound_device> m_speaker;
	optional_ioport_array<8+1> m_inputs; // max 8
	optional_ioport m_io_ba, m_io_b;
	output_finder<16, 16, 4> m_out_x;

	// misc common
	u16 m_inp_mux = 0;      // multiplexed inputs mask
	int m_inp_lines = 0;    // number of input mux columns
	int m_inp_fixed = -1;   // input column fixed to GND/Vdd (-1 means none)
	u8 m_speaker_data = 0;  // speaker output data(if more than 1 bit)
	u8 m_s = 0;             // MCU S output pins
	u8 m_r = 0;             // MCU R output pins

	void inp_fixed_last() { m_inp_fixed = -2; } // last input line to GND
	u8 read_inputs(int columns, int fixed = -1);

	virtual void update_k_line();
	virtual void sm510_lcd_segment_w(offs_t offset, u16 data);
	virtual void sm500_lcd_segment_w(offs_t offset, u16 data);
	virtual u8 input_r();
	virtual void input_w(u8 data);
	virtual void piezo_r1_w(u8 data);
	virtual void piezo_r2_w(u8 data);
	virtual void piezo_input_w(u8 data);
	virtual void piezo2bit_r1_w(u8 data);
	virtual void piezo2bit_input_w(u8 data);

	// display common
	int m_decay_pivot = 8;                // lcd segment off-to-on delay in 1kHz ticks (affects input lag)
	int m_decay_len = 17;                 // lcd segment on-to-off delay in 1kHz ticks (lcd persistence)
	u8 m_display_x_len = 0;               // lcd number of groups
	u8 m_display_y_len = 0;               // lcd number of segments
	u8 m_display_z_len = 0;               // lcd number of commons
	u32 m_display_state[0x20] = { };      // lcd segment data (max. 5-bit offset)
	u8 m_display_decay[0x20][0x20] = { }; // (internal use)

	void set_display_size(u8 x, u8 y, u8 z);
	TIMER_CALLBACK_MEMBER(display_decay_tick);
	emu_timer *m_display_decay_timer = nullptr;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// machine configs
	void mcfg_cpu_common(machine_config &config);
	void mcfg_cpu_sm5a(machine_config &config);
	void mcfg_cpu_kb1013vk12(machine_config &config);
	void mcfg_cpu_sm510(machine_config &config);
	void mcfg_cpu_sm511(machine_config &config);
	void mcfg_cpu_sm512(machine_config &config);
	void mcfg_cpu_sm530(machine_config &config);
	void mcfg_svg_screen(machine_config &config, u16 width, u16 height, const char *tag = "screen");
	void mcfg_sound_r1(machine_config &config);

	void sm5a_common(machine_config &config, u16 width, u16 height);
	void kb1013vk12_common(machine_config &config, u16 width, u16 height);
	void sm510_common(machine_config &config, u16 width, u16 height);
	void sm511_common(machine_config &config, u16 width, u16 height);
	//void sm512_common(machine_config &config, u16 width, u16 height);
	void sm530_common(machine_config &config, u16 width, u16 height);

	void sm510_dualh(machine_config &config, u16 leftwidth, u16 leftheight, u16 rightwidth, u16 rightheight);
	void dualv_common(machine_config &config, u16 topwidth, u16 topheight, u16 botwidth, u16 botheight);
	void sm510_dualv(machine_config &config, u16 topwidth, u16 topheight, u16 botwidth, u16 botheight);
	void sm511_dualv(machine_config &config, u16 topwidth, u16 topheight, u16 botwidth, u16 botheight);
	void sm512_dualv(machine_config &config, u16 topwidth, u16 topheight, u16 botwidth, u16 botheight);
	void sm510_tiger(machine_config &config, u16 width, u16 height);
	void sm511_tiger1bit(machine_config &config, u16 width, u16 height);
	void sm511_tiger2bit(machine_config &config, u16 width, u16 height);
};


#endif // MAME_HANDHELD_HH_SM510_H
