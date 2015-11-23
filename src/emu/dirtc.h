// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    dirtc.h

    Device Real Time Clock interfaces.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DIRTC_H__
#define __DIRTC_H__



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

// clock registers
enum
{
	RTC_SECOND = 0,
	RTC_MINUTE,
	RTC_HOUR,
	RTC_DAY,
	RTC_MONTH,
	RTC_DAY_OF_WEEK,
	RTC_YEAR
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_rtc_interface

// class representing interface-specific live rtc
class device_rtc_interface : public device_interface
{
public:
	// construction/destruction
	device_rtc_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_rtc_interface();

	void set_time(bool update, int year, int month, int day, int day_of_week, int hour, int minute, int second);
	void set_current_time(running_machine &machine);

protected:
	static UINT8 convert_to_bcd(int val);
	static int bcd_to_integer(UINT8 val);

	void set_clock_register(int register, int value);
	int get_clock_register(int register);
	void clock_updated();

	void advance_seconds();
	void advance_minutes();
	void advance_days();
	void adjust_seconds();

	// derived class overrides
	virtual bool rtc_feature_y2k() { return false; }
	virtual bool rtc_feature_leap_year() { return false; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) { }

	int m_register[7];
};


#endif  /* __DIRTC_H__ */
