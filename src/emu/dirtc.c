// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    dirtc.c

    Device Real Time Clock interfaces.

***************************************************************************/

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

static const int DAYS_PER_MONTH[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };



//**************************************************************************
//  DEVICE RTC INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_rtc_interface - constructor
//-------------------------------------------------

device_rtc_interface::device_rtc_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "rtc")
{
}


//-------------------------------------------------
//  ~device_rtc_interface - destructor
//-------------------------------------------------

device_rtc_interface::~device_rtc_interface()
{
}


//-------------------------------------------------
//  set_time - called to initialize the RTC to
//  a known date/time
//-------------------------------------------------

void device_rtc_interface::set_time(bool update, int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	if (!rtc_feature_y2k())
	{
		year %= 100;
	}

	set_clock_register(RTC_YEAR, year);
	set_clock_register(RTC_MONTH, month);
	set_clock_register(RTC_DAY, day);
	set_clock_register(RTC_DAY_OF_WEEK, day_of_week);
	set_clock_register(RTC_HOUR, hour);
	set_clock_register(RTC_MINUTE, minute);
	set_clock_register(RTC_SECOND, second);

	if (update)
	{
		rtc_clock_updated(m_register[RTC_YEAR], m_register[RTC_MONTH], m_register[RTC_DAY], m_register[RTC_DAY_OF_WEEK],
			m_register[RTC_HOUR], m_register[RTC_MINUTE], m_register[RTC_SECOND]);
	}
}


//-------------------------------------------------
//  set_current_time - called to initialize the RTC
//  to the current system time
//-------------------------------------------------

void device_rtc_interface::set_current_time(running_machine &machine)
{
	system_time systime;
	machine.base_datetime(systime);

	set_time(true, systime.local_time.year, systime.local_time.month + 1, systime.local_time.mday, systime.local_time.weekday + 1,
		systime.local_time.hour, systime.local_time.minute, systime.local_time.second);
}


//-------------------------------------------------
//  convert_to_bcd -
//-------------------------------------------------

UINT8 device_rtc_interface::convert_to_bcd(int val)
{
	return ((val / 10) << 4) | (val % 10);
}


//-------------------------------------------------
//  bcd_to_integer -
//-------------------------------------------------

int device_rtc_interface::bcd_to_integer(UINT8 val)
{
	return (((val & 0xf0) >> 4) * 10) + (val & 0x0f);
}


//-------------------------------------------------
//  set_clock_register -
//-------------------------------------------------

void device_rtc_interface::set_clock_register(int reg, int value)
{
	m_register[reg] = value;
}


//-------------------------------------------------
//  get_clock_register -
//-------------------------------------------------

int device_rtc_interface::get_clock_register(int reg)
{
	return m_register[reg];
}


//-------------------------------------------------
//  clock_updated -
//-------------------------------------------------

void device_rtc_interface::clock_updated()
{
	rtc_clock_updated(m_register[RTC_YEAR], m_register[RTC_MONTH], m_register[RTC_DAY], m_register[RTC_DAY_OF_WEEK],
		m_register[RTC_HOUR], m_register[RTC_MINUTE], m_register[RTC_SECOND]);
}


//-------------------------------------------------
//  advance_seconds -
//-------------------------------------------------

void device_rtc_interface::advance_seconds()
{
	m_register[RTC_SECOND]++;

	if (m_register[RTC_SECOND] == 60)
	{
		m_register[RTC_SECOND] = 0;

		advance_minutes();
	}
	else
	{
		clock_updated();
	}
}


//-------------------------------------------------
//  advance_minutes
//-------------------------------------------------

void device_rtc_interface::advance_minutes()
{
	m_register[RTC_MINUTE]++;

	if (m_register[RTC_MINUTE] == 60)
	{
		m_register[RTC_MINUTE] = 0;
		m_register[RTC_HOUR]++;
	}

	if (m_register[RTC_HOUR] == 24)
	{
		m_register[RTC_HOUR] = 0;
		advance_days();
	}
	else
	{
		clock_updated();
	}
}


//-------------------------------------------------
//  advance_days
//-------------------------------------------------

void device_rtc_interface::advance_days()
{
	m_register[RTC_DAY]++;
	m_register[RTC_DAY_OF_WEEK]++;

	if (m_register[RTC_DAY_OF_WEEK] == 8)
	{
		m_register[RTC_DAY_OF_WEEK] = 1;
	}

	if (m_register[RTC_DAY] > DAYS_PER_MONTH[m_register[RTC_MONTH] - 1])
	{
		m_register[RTC_DAY] = 1;
		m_register[RTC_MONTH]++;
	}

	if (m_register[RTC_MONTH] == 13)
	{
		m_register[RTC_MONTH] = 1;
		m_register[RTC_YEAR]++;

		if (!rtc_feature_y2k() && m_register[RTC_YEAR] == 100)
		{
			m_register[RTC_YEAR] = 0;
		}
	}

	clock_updated();
}


//-------------------------------------------------
//  adjust_seconds -
//-------------------------------------------------

void device_rtc_interface::adjust_seconds()
{
	int seconds = get_clock_register(RTC_SECOND);

	set_clock_register(RTC_SECOND, 0);

	if (seconds >= 30)
	{
		advance_minutes();
	}
	else
	{
		clock_updated();
	}
}
