// license:BSD-3-Clause
// copyright-holders:hap
/*

  National Semiconductor MM5445 series VFD Driver

*/

#ifndef MAME_VIDEO_MM5445_H
#define MAME_VIDEO_MM5445_H

#pragma once

// pinout reference

/*
             ____   ____
     VDD  1 |*   \_/    | 40 O18
     O17  2 |           | 39 O19
     O16  3 |           | 38 O20
     O15  4 |           | 37 O21
     O14  5 |           | 36 O22
     O13  6 |           | 35 O23
     O12  7 |           | 34 O24
     O11  8 |           | 33 O25
     O10  9 |           | 32 O26
      O9 10 |  MM5445N  | 31 O27
      O8 11 |           | 30 O28
      O7 12 |           | 29 O29
      O6 13 |           | 28 O30
      O5 14 |           | 27 O31
      O4 15 |           | 26 O32
      O3 16 |           | 25 O33
      O2 17 |           | 24 BRIGHTNESS CONTROL
      O1 18 |           | 23 _DATA ENABLE
     VGG 19 |           | 22 DATA IN
     VSS 20 |___________| 21 CLOCK IN

    O# = OUTPUT BIT #
    MM5446, MM5448 don't have the brightness control pin, an extra output pin instead
    MM5447, MM5448 don't have the data enable pin(always enabled), but another extra output pin
*/


class mm5445_device : public device_t
{
public:
	mm5445_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// configuration helpers
	auto output_cb() { return m_write_output.bind(); }

	void clock_w(int state);
	void enable_w(int state) { m_enable = (state) ? 1 : 0; } // active low, unused on MM5447 and MM5448
	void data_w(int state) { m_data = (state) ? 1 : 0; }

protected:
	mm5445_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 outpins);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	const u64 m_outmask;

	int m_clk;      // input pin state
	int m_enable;   // "
	int m_data;     // "

	u64 m_shiftreg;
	int m_shiftcount;

	// callbacks
	devcb_write64 m_write_output;
};


class mm5446_device : public mm5445_device
{
public:
	mm5446_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class mm5447_device : public mm5445_device
{
public:
	mm5447_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class mm5448_device : public mm5445_device
{
public:
	mm5448_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(MM5445, mm5445_device)
DECLARE_DEVICE_TYPE(MM5446, mm5446_device)
DECLARE_DEVICE_TYPE(MM5447, mm5447_device)
DECLARE_DEVICE_TYPE(MM5448, mm5448_device)

#endif // MAME_VIDEO_MM5445_H
