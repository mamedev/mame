// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************
*
*  CXG chess computer driver base class
*
******************************************************************************/

#pragma once

#ifndef MAME_INCLUDES_CXGBASE_H
#define MAME_INCLUDES_CXGBASE_H

#include "machine/timer.h"
#include "sound/dac.h"

class cxgbase_state : public driver_device
{
public:
	cxgbase_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_irq_on(*this, "irq_on"),
		m_dac(*this, "dac"),
		m_inp_matrix(*this, "IN.%u", 0),
		m_out_x(*this, "%u.%u", 0U, 0U),
		m_out_a(*this, "%u.a", 0U),
		m_out_digit(*this, "digit%u", 0U),
		m_display_wait(33),
		m_display_maxy(1),
		m_display_maxx(0)
	{ }

	// devices/pointers
	required_device<cpu_device> m_maincpu;
	optional_device<timer_device> m_irq_on;
	optional_device<dac_bit_interface> m_dac;
	optional_ioport_array<10> m_inp_matrix; // max 10
	output_finder<0x20, 0x20> m_out_x;
	output_finder<0x20> m_out_a;
	output_finder<0x20> m_out_digit;

	// misc common
	u16 m_inp_mux;                  // multiplexed keypad mask
	u16 m_led_select;
	u16 m_led_data;

	u16 read_inputs(int columns);

	// periodic interrupts
	template<int Line> TIMER_DEVICE_CALLBACK_MEMBER(irq_on) { m_maincpu->set_input_line(Line, ASSERT_LINE); }
	template<int Line> TIMER_DEVICE_CALLBACK_MEMBER(irq_off) { m_maincpu->set_input_line(Line, CLEAR_LINE); }

	// display common
	int m_display_wait;             // led/lamp off-delay in milliseconds (default 33ms)
	int m_display_maxy;             // display matrix number of rows
	int m_display_maxx;             // display matrix number of columns (max 31 for now)

	u32 m_display_state[0x20];      // display matrix rows data (last bit is used for always-on)
	u16 m_display_segmask[0x20];    // if not 0, display matrix row is a digit, mask indicates connected segments
	u8 m_display_decay[0x20][0x20]; // (internal use)

	TIMER_DEVICE_CALLBACK_MEMBER(display_decay_tick);
	void display_update();
	void set_display_size(int maxx, int maxy);
	void display_matrix(int maxx, int maxy, u32 setx, u32 sety, bool update = true);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


INPUT_PORTS_EXTERN( cxg_cb_magnets );

#endif // MAME_INCLUDES_CXGBASE_H
