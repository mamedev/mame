// license:BSD-3-Clause
// copyright-holders:Raphael Nabet,Sergey Svishchev

#ifndef MAME_MACHINE_MM58174_H
#define MAME_MACHINE_MM58174_H

#pragma once

#include "dirtc.h"

/***************************************************************************
    MACROS
***************************************************************************/

class mm58174_device : public device_t, public device_rtc_interface
{
public:
	mm58174_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_rtc_interface overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;
	virtual bool rtc_feature_leap_year() const override { return true; }

	TIMER_CALLBACK_MEMBER(clock_tick);
	TIMER_CALLBACK_MEMBER(scheduler_sync);

	void update_rtc();

private:
	// internal state

	attotime interrupt_period_table(int val);

	uint8_t m_control;    // control register (write to address 14)

	uint8_t m_int_ctl;    // interrupt control register

	uint8_t m_years;      // a year that maps to leap year setting
	uint8_t m_wday;       // day of the week (1-7 (1=day1 as set in init))
	uint8_t m_months1;    // months (BCD: 1-12)
	uint8_t m_months2;
	uint8_t m_days1;      // days (BCD: 1-31)
	uint8_t m_days2;
	uint8_t m_hours1;     // hours (BCD : 0-23)
	uint8_t m_hours2;
	uint8_t m_minutes1;   // minutes (BCD : 0-59)
	uint8_t m_minutes2;
	uint8_t m_seconds1;   // seconds (BCD : 0-59)
	uint8_t m_seconds2;
	uint8_t m_tenths;     // tenths of second (BCD : 0-9)

	emu_timer *m_rtc_timer;
	emu_timer *m_interrupt_timer;
};

DECLARE_DEVICE_TYPE(MM58174, mm58174_device)

#endif // MAME_MACHINE_MM58174_H
