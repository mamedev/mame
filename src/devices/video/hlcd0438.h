// license:BSD-3-Clause
// copyright-holders:hap
/*

  Hughes HLCD 0438 LCD Driver

*/

#ifndef MAME_VIDEO_HLCD0438_H
#define MAME_VIDEO_HLCD0438_H

#pragma once

// pinout reference

/*
               ____   ____
       VDD  1 |*   \_/    | 40 CLOCK
      LOAD  2 |           | 39 SEG 1
    SEG 32  3 |           | 38 SEG 2
    SEG 31  4 |           | 37 SEG 3
    SEG 30  5 |           | 36 GND
    SEG 29  6 |           | 35 DATA OUT
    SEG 28  7 |           | 34 DATA IN
    SEG 27  8 |           | 33 SEG 4
    SEG 26  9 |           | 32 SEG 5
    SEG 25 10 | HLCD 0438 | 31 LCD
    SEG 24 11 |           | 30 BP
    SEG 23 12 |           | 29 SEG 6
    SEG 22 13 |           | 28 SEG 7
    SEG 21 14 |           | 27 SEG 8
    SEG 20 15 |           | 26 SEG 9
    SEG 19 16 |           | 25 SEG 10
    SEG 18 17 |           | 24 SEG 11
    SEG 17 18 |           | 23 SEG 12
    SEG 16 19 |           | 22 SEG 13
    SEG 15 20 |___________| 21 SEG 14

*/


class hlcd0438_device : public device_t
{
public:
	hlcd0438_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto write_segs() { return m_write_segs.bind(); } // BP pin in offset, SEG pins in data
	auto write_data() { return m_write_data.bind(); } // DATA OUT pin
	hlcd0438_device &set_load(int state) { m_load = state ? 1 : 0; return *this; } // if hardwired, can just set LOAD pin state here

	void data_w(int state) { m_data_in = state ? 1 : 0; }
	int data_r() { return m_data_out; }
	void clock_w(int state);
	void load_w(int state);
	void lcd_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(toggle_lcd);

private:
	emu_timer *m_lcd_timer;

	// pin state
	int m_data_in;
	int m_data_out;
	int m_clk;
	int m_load;
	int m_lcd;

	u32 m_shift;
	u32 m_latch;

	devcb_write32 m_write_segs;
	devcb_write_line m_write_data;
};


DECLARE_DEVICE_TYPE(HLCD0438, hlcd0438_device)

#endif // MAME_VIDEO_HLCD0438_H
