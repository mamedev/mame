// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM5xx family handhelds.

*/

#ifndef MAME_INCLUDES_HH_SM510_H
#define MAME_INCLUDES_HH_SM510_H

#include "cpu/sm510/sm510.h"
#include "sound/spkrdev.h"


class hh_sm510_state : public driver_device
{
public:
	hh_sm510_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker"),
		m_inputs(*this, "IN.%u", 0),
		m_out_x(*this, "%u.%u.%u", 0U, 0U, 0U),
		m_inp_lines(0),
		m_inp_fixed(-1),
		m_decay_pivot(7),
		m_decay_len(17)
	{ }

	virtual DECLARE_INPUT_CHANGED_MEMBER(input_changed);
	virtual DECLARE_INPUT_CHANGED_MEMBER(acl_button);

protected:
	// devices
	required_device<sm510_base_device> m_maincpu;
	optional_device<speaker_sound_device> m_speaker;
	optional_ioport_array<8+1> m_inputs; // max 8
	output_finder<16, 16, 4> m_out_x;

	// misc common
	u16 m_inp_mux;                  // multiplexed inputs mask
	int m_inp_lines;                // number of input mux columns
	int m_inp_fixed;                // input column fixed to GND/Vdd (-1 means none)
	u8 m_speaker_data;              // speaker output data(if more than 1 bit)
	u8 m_s;                         // MCU S output pins
	u8 m_r;                         // MCU R output pins

	void inp_fixed_last() { m_inp_fixed = -2; } // last input line to GND
	u8 read_inputs(int columns, int fixed = -1);

	virtual void update_k_line();
	virtual DECLARE_WRITE16_MEMBER(sm510_lcd_segment_w);
	virtual DECLARE_WRITE16_MEMBER(sm500_lcd_segment_w);
	virtual DECLARE_READ8_MEMBER(input_r);
	virtual DECLARE_WRITE8_MEMBER(input_w);
	virtual DECLARE_WRITE8_MEMBER(piezo_r1_w);
	virtual DECLARE_WRITE8_MEMBER(piezo_r2_w);
	virtual DECLARE_WRITE8_MEMBER(piezo_input_w);
	virtual DECLARE_WRITE8_MEMBER(piezo2bit_r1_w);
	virtual DECLARE_WRITE8_MEMBER(piezo2bit_input_w);

	// display common
	int m_decay_pivot;              // lcd segment off-to-on delay in 1kHz ticks (affects input lag)
	int m_decay_len;                // lcd segment on-to-off delay in 1kHz ticks (lcd persistence)
	u8 m_display_x_len;             // lcd number of groups
	u8 m_display_y_len;             // lcd number of segments
	u8 m_display_z_len;             // lcd number of commons
	u32 m_display_state[0x20];      // lcd segment data (max. 5-bit offset)
	u8 m_display_decay[0x20][0x20]; // (internal use)

	void set_display_size(u8 x, u8 y, u8 z);
	TIMER_CALLBACK_MEMBER(display_decay_tick);
	emu_timer *m_display_decay_timer;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void common_base(machine_config &config, u16 width, u16 height);
	void sm500_base(machine_config &config, u16 width, u16 height);
	void sm510_base(machine_config &config, u16 width, u16 height);

	void common_sm511(machine_config &config, u16 width, u16 height);

	void gnw_sm5a(machine_config &config, u16 width, u16 height);
	void gnw_sm5a_matrix(machine_config &config, u16 width, u16 height);
	void gnw_kb1013vk12_matrix(machine_config &config, u16 width, u16 height);
	void gnw_sm510(machine_config &config, u16 width, u16 height);
	void gnw_sm511(machine_config &config, u16 width, u16 height);
	void gnw_dualh(machine_config &config, u16 leftwidth, u16 leftheight, u16 rightwidth, u16 rightheight);
	void gnw_dualv(machine_config &config, u16 topwidth, u16 topheight, u16 botwidth, u16 botheight);
	void gnw_sm510_dualh(machine_config &config, u16 leftwidth, u16 leftheight, u16 rightwidth, u16 rightheight);
	void gnw_sm510_dualv(machine_config &config, u16 topwidth, u16 topheight, u16 botwidth, u16 botheight);
	void gnw_sm511_dualv(machine_config &config, u16 topwidth, u16 topheight, u16 botwidth, u16 botheight);
	void gnw_sm512_dualv(machine_config &config, u16 topwidth, u16 topheight, u16 botwidth, u16 botheight);

	void konami_sm510(machine_config &config, u16 width, u16 height);

	void tiger_sm510_1bit(machine_config &config, u16 width, u16 height);
	void tiger_sm511_1bit(machine_config &config, u16 width, u16 height);
	void tiger_sm511_2bit(machine_config &config, u16 width, u16 height);
};


#endif // MAME_INCLUDES_HH_SM510_H
