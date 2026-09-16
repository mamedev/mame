// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    dirtc.h

    Device Real Time Clock interfaces.

***************************************************************************/

#ifndef MAME_EMU_DIRTC_H
#define MAME_EMU_DIRTC_H

#pragma once



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

	// use UTC instead of local time when syncing at startup
	void set_use_utc(bool use_utc) { m_use_utc = use_utc; }

	void set_time(bool update, int year, int month, int day, int day_of_week, int hour, int minute, int second);
	void set_current_time(const system_time &systime);

	bool has_battery() const { return rtc_battery_backed(); }

protected:
	static u8 convert_to_bcd(int val);
	static int bcd_to_integer(u8 val);

	void set_clock_register(int reg, int value);
	int get_clock_register(int reg);
	void clock_updated();

	void advance_seconds();
	void advance_minutes();
	void advance_days();
	void adjust_seconds();

	// derived class overrides
	virtual bool rtc_feature_y2k() const { return false; }
	virtual bool rtc_feature_leap_year() const { return false; }
	virtual bool rtc_battery_backed() const { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) = 0;

	int m_register[7];

	bool m_use_utc;
};

// iterator
typedef device_interface_enumerator<device_rtc_interface> rtc_interface_enumerator;


#endif // MAME_EMU_DIRTC_H
