// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************
*
*  Generic chess computers base driver
*  implementation is in machine/chessbase.cpp
*
******************************************************************************/

#ifndef MAME_INCLUDES_CHESSBASE_H
#define MAME_INCLUDES_CHESSBASE_H

#pragma once

#include "machine/timer.h"

class chessbase_state : public driver_device
{
public:
	chessbase_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_inp_matrix(*this, "IN.%u", 0),
		m_out_x(*this, "%u.%u", 0U, 0U),
		m_out_a(*this, "%u.a", 0U),
		m_out_digit(*this, "digit%u", 0U),
		m_display_wait(33),
		m_display_maxy(1),
		m_display_maxx(0)
	{ }

protected:
	// devices/pointers
	optional_ioport_array<16> m_inp_matrix; // max 16
	output_finder<0x20, 0x20> m_out_x;
	output_finder<0x20> m_out_a;
	output_finder<0x20> m_out_digit;

	// misc common
	u16 m_inp_mux;                  // multiplexed keypad/leds mask
	u16 m_led_select;
	u16 m_led_data;
	u16 m_led_latch;
	u32 m_7seg_data;                // data for seg leds

	u16 read_inputs(int columns);

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
	void set_display_segmask(u32 digits, u32 mask);
	void display_matrix(int maxx, int maxy, u32 setx, u32 sety, bool update = true);

	virtual void machine_start() override;
	virtual void machine_reset() override;
};


INPUT_PORTS_EXTERN( generic_cb_buttons );
INPUT_PORTS_EXTERN( generic_cb_magnets );

#endif // MAME_INCLUDES_CHESSBASE_H
