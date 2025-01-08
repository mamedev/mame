// license:BSD-3-Clause
// copyright-holders:Curt Coder, hap
/**********************************************************************

    NEC uPD1990AC Serial I/O Calendar & Clock emulation

**********************************************************************
                            _____   _____
                    C2   1 |*    \_/     | 14  Vdd
                    C1   2 |             | 13  XTAL
                    C0   3 |             | 12  _XTAL
                   STB   4 |  uPD1990AC  | 11  OUT ENBL
                    CS   5 |             | 10  TP
               DATA IN   6 |             | 9   DATA OUT
                   GND   7 |_____________| 8   CLK

**********************************************************************/

#ifndef MAME_MACHINE_UPD1990A_H
#define MAME_MACHINE_UPD1990A_H

#pragma once

#include "dirtc.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> upd1990a_device

class upd1990a_device : public device_t, public device_rtc_interface
{
public:
	// construction/destruction
	upd1990a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 32'768);

	auto data_callback() { return m_write_data.bind(); }
	auto tp_callback() { return m_write_tp.bind(); }

	void oe_w(int state);
	void cs_w(int state);
	void stb_w(int state);
	void clk_w(int state);
	void c0_w(int state);
	void c1_w(int state);
	void c2_w(int state);
	void data_in_w(int state);
	int data_out_r();
	int tp_r();

protected:
	// device-level overrides
	upd1990a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant);

	virtual void device_start() override ATTR_COLD;

	// device_rtc_interface overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	TIMER_CALLBACK_MEMBER(clock_tick);
	TIMER_CALLBACK_MEMBER(tp_tick);
	TIMER_CALLBACK_MEMBER(data_out_tick);
	TIMER_CALLBACK_MEMBER(test_tick);

	enum
	{
		TYPE_1990A = 0,
		TYPE_4990A
	};

private:
	enum
	{
		MODE_REGISTER_HOLD = 0,
		MODE_SHIFT,
		MODE_TIME_SET,
		MODE_TIME_READ,
		MODE_TP_64HZ,
		MODE_TP_256HZ,
		MODE_TP_2048HZ,
		MODE_TP_4096HZ,
		MODE_TP_1S_INT,
		MODE_TP_10S_INT,
		MODE_TP_30S_INT,
		MODE_TP_60S_INT,
		MODE_INT_RESET_OUTPUT,
		MODE_INT_RUN_CLOCK,
		MODE_INT_STOP_CLOCK,
		MODE_TEST
	};

	devcb_write_line m_write_data;
	devcb_write_line m_write_tp;

	uint8_t m_time_counter[6];    // time counter
	uint8_t m_shift_reg[7];       // shift register (40 bits, or 48 bits + serial command register)

	int m_oe;                   // output enable
	int m_cs;                   // chip select
	int m_stb;                  // strobe
	int m_data_in;              // data in
	int m_data_out;             // data out
	int m_c;                    // latched command
	int m_clk;                  // shift clock
	int m_tp;                   // time pulse
	int m_c_unlatched;          // command waiting for STB

	bool m_testmode;            // testmode active

	int const m_variant;

	// timers
	emu_timer *m_timer_clock;
	emu_timer *m_timer_tp;
	emu_timer *m_timer_data_out;
	emu_timer *m_timer_test_mode;

	bool is_serial_mode();
	int get_data_out();
};


// ======================> upd4990a_device

class upd4990a_device : public upd1990a_device
{
public:
	upd4990a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 32'768);
};


// device type definitions
DECLARE_DEVICE_TYPE(UPD1990A, upd1990a_device)
DECLARE_DEVICE_TYPE(UPD4990A, upd4990a_device)

#endif // MAME_MACHINE_UPD1990A_H
