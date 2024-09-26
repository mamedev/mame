// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Microelectronic-Marin E050-16 Real Time Clock emulation

**********************************************************************
                            _____   _____
                  Vdd1   1 |*    \_/     | 16  Vdd2
                OSC IN   2 |             | 15  Clk
               OSC OUT   3 |             | 14  XOUT
                 _STOP   4 |   E05-16    | 13  DI/O
                _RESET   5 |   E050-16   | 12  _SEC
               _OUTSEL   6 |             | 11  _MIN
                  _DAY   7 |             | 10  _HRS
                   Vss   8 |_____________| 9   _CS

**********************************************************************/

#ifndef MAME_MACHINE_E0516_H
#define MAME_MACHINE_E0516_H

#pragma once

#include "dirtc.h"

class e0516_device : public device_t, public device_rtc_interface
{
public:
	// construction/destruction
	e0516_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto outsel_rd_cb() { return m_read_outsel.bind(); }
	auto sec_wr_cb() { return m_write_sec.bind(); }
	auto min_wr_cb() { return m_write_min.bind(); }
	auto hrs_wr_cb() { return m_write_hrs.bind(); }
	auto day_wr_cb() { return m_write_day.bind(); }

	void cs_w(int state);
	void clk_w(int state);
	void dio_w(int state);
	int dio_r();
	void reset_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_rtc_interface overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	TIMER_CALLBACK_MEMBER(timer_tick);

	devcb_read_line m_read_outsel;
	devcb_write_line m_write_sec;
	devcb_write_line m_write_min;
	devcb_write_line m_write_hrs;
	devcb_write_line m_write_day;

private:
	bool m_cs;
	bool m_clk;
	int m_cycle;
	u64 m_data_latch;
	int m_reg_latch;
	int m_state;
	int m_bits_left;
	bool m_dio;
	bool m_reset;

	offs_t get_address() { return (m_reg_latch >> 1) & 0x07; }

	// timers
	emu_timer *m_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(E0516, e0516_device)

#endif // MAME_MACHINE_E0516_H
