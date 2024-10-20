// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
/*

  Hughes HLCD 0488 LCD Driver

*/

#ifndef MAME_VIDEO_HLCD0488_H
#define MAME_VIDEO_HLCD0488_H

#pragma once

// pinout reference

/*
                 ____   ____
       ROW 4  1 |*   \_/    | 40 VDD
       ROW 5  2 |           | 39 ROW 1
       ROW 6  3 |           | 38 ROW 2
   /DATA CLK  4 |           | 37 ROW 3
 LATCH PULSE  5 |           | 36 COL 1
      DATA 0  6 |           | 35 COL 2
      DATA 1  7 |           | 34 COL 3
      DATA 2  8 |           | 33 COL 4
      DATA 3  9 |           | 32 COL 5
      ROW 16 10 | HLCD 0488 | 31 COL 6
      ROW 15 11 |           | 30 COL 7
      ROW 14 12 |           | 29 COL 8
      ROW 13 13 |           | 28 COL 9
      ROW 12 14 |           | 27 COL 10
      ROW 11 15 |           | 26 COL 11
      ROW 10 16 |           | 25 COL 12
       ROW 9 17 |           | 24 COL 13
       ROW 8 18 |           | 23 COL 14
       ROW 7 19 |           | 22 COL 15
         GND 20 |___________| 21 COL 16

*/


class hlcd0488_device : public device_t
{
public:
	hlcd0488_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// configuration helpers
	auto write_cols() { return m_write_cols.bind(); } // COL pins in data, ROW pins in offset

	void latch_pulse_w(int state);
	void data_clk_w(int state);
	void data_w(u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(sync_update);

private:
	// pin state
	u8 m_latch_pulse;
	u8 m_latch_pulse_prev;
	u8 m_data_clk;
	u8 m_data_clk_prev;
	u8 m_data;

	u8 m_count;
	u8 m_latch[8];
	u8 m_hold[8];

	emu_timer *m_sync_timer;
	devcb_write16 m_write_cols;
};


DECLARE_DEVICE_TYPE(HLCD0488, hlcd0488_device)

#endif // MAME_VIDEO_HLCD0488_H
