// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Mullard SAB3019 Clock/Timer with Serial I/O emulation

**********************************************************************
                            _____   _____
                  NODA   1 |*    \_/     | 16  Vdd
                   F50   2 |             | 15  Vss1
                  COMP   3 |             | 14  OSCO
                  DATA   4 |   SAB3019   | 13  OSCI
                  DLEN   5 |             | 12  TEST
                   CLB   6 |             | 11  FSET
                  POWF   7 |             | 10  SEC
                  Vss2   8 |_____________| 9   MIN

**********************************************************************/

#ifndef MAME_MACHINE_SAB3019_H
#define MAME_MACHINE_SAB3019_H

#pragma once

#include "dirtc.h"


class sab3019_device : public device_t, public device_rtc_interface
{
public:
	sab3019_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 32'768);

	auto comp_callback() { return m_write_comp.bind(); }
	auto sec_callback() { return m_write_sec.bind(); }
	auto min_callback() { return m_write_min.bind(); }

	void dlen_w(int state);
	void clb_w(int state);
	void data_w(int state);
	int data_r();
	void noda_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_rtc_interface implementation
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

private:
	enum
	{
		MONTH = 0,
		DATE,
		HOURS,
		MINUTES
	};

	devcb_write_line m_write_comp;
	devcb_write_line m_write_sec;
	devcb_write_line m_write_min;

	emu_timer *m_prescaler_timer;

	uint8_t m_counter[4];
	uint8_t m_register[4];
	bool m_daily;
	uint8_t m_seconds;
	bool m_half;

	bool m_dlen;
	bool m_clb;
	bool m_data_in;
	bool m_noda;
	bool m_comp;

	uint8_t m_bit_count;
	uint16_t m_shift;
	bool m_load;
	bool m_read_pending;
	bool m_reading;
	bool m_read_s;
	uint8_t m_read_addr;
	uint8_t m_read_data;
	uint8_t m_read_bit;
	bool m_minute_flag;

	TIMER_CALLBACK_MEMBER(prescaler_tick);

	void reset_prescaler();
	void reset_seconds();
	void advance_minute();
	void update_comp();
	void execute(uint8_t count, uint16_t bits);
	uint8_t read_digits(bool s, uint8_t addr) const;
};


DECLARE_DEVICE_TYPE(SAB3019, sab3019_device)

#endif // MAME_MACHINE_SAB3019_H
