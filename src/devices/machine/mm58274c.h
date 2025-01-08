// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
#ifndef MAME_MACHINE_MM58274C_H
#define MAME_MACHINE_MM58274C_H

#include "dirtc.h"

/***************************************************************************
    MACROS
***************************************************************************/

class mm58274c_device : public device_t, public device_rtc_interface
{
public:
	mm58274c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_mode24(int mode) { m_mode24 = mode; }
	void set_day1(int day) { m_day1 = day; }
	void set_mode_and_day(int mode, int day) { m_mode24 = mode; m_day1 = day; }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	TIMER_CALLBACK_MEMBER(rtc_increment_cb);
	TIMER_CALLBACK_MEMBER(rtc_interrupt_cb);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_rtc_interface overrides
	virtual bool rtc_feature_y2k() const override { return false; }
	virtual bool rtc_feature_leap_year() const override { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

private:
	// internal state

	// Initialization of the clock chip:
	// m_day1 must be set to a value from 0 (sunday), 1 (monday)...
	// to 6 (saturday) and is needed to correctly retrieve the
	// day-of-week from the host system clock.
	int m_mode24;     /* 24/12 mode */
	int m_day1;       /* first day of week */

	attotime interrupt_period_table(int val);

	int m_status;     /* status register (*read* from address 0 = control register) */
	int m_control;    /* control register (*write* to address 0) */

	int m_clk_set;    /* clock setting register */
	int m_int_ctl;    /* interrupt control register */

	int m_wday;       /* day of the week (1-7 (1=day1 as set in init)) */
	int m_years1;     /* years (BCD: 0-99) */
	int m_years2;
	int m_months1;    /* months (BCD: 1-12) */
	int m_months2;
	int m_days1;      /* days (BCD: 1-31) */
	int m_days2;
	int m_hours1;     /* hours (BCD : 0-23) */
	int m_hours2;
	int m_minutes1;   /* minutes (BCD : 0-59) */
	int m_minutes2;
	int m_seconds1;   /* seconds (BCD : 0-59) */
	int m_seconds2;
	int m_tenths;     /* tenths of second (BCD : 0-9) */

	emu_timer *m_increment_rtc;
	emu_timer *m_interrupt_timer;
};

DECLARE_DEVICE_TYPE(MM58274C, mm58274c_device)

#endif // MAME_MACHINE_MM58274C_H
