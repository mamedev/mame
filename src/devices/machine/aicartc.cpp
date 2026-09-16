// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    AICA-RTC sub-device

    TODO:
    - move this inside AICA sound core once that'll get modernized

***************************************************************************/

#include "emu.h"
#include "aicartc.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(AICARTC, aicartc_device, "aicartc", "AICA RTC")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  aicartc_device - constructor
//-------------------------------------------------

aicartc_device::aicartc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AICARTC, tag, owner, clock)
	, device_rtc_interface(mconfig, *this)
	, m_rtc_reg_lo(0), m_rtc_reg_hi(0), m_rtc_tick(0), m_we(0)
	, m_clock_timer(nullptr)
{
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void aicartc_device::device_validity_check(validity_checker &valid) const
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void aicartc_device::device_start()
{
	m_clock_timer = timer_alloc(FUNC(aicartc_device::clock_tick), this);
	m_clock_timer->adjust(attotime::from_hz(clock()), 0, attotime::from_hz(clock()));
}


//-------------------------------------------------
//  rtc_clock_updated -
//-------------------------------------------------

void aicartc_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	const int month_to_day_conversion[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

	// put the seconds
	uint32_t current_time = second;

	// put the minutes
	current_time += minute * 60;

	// put the hours
	current_time += hour * 60 * 60;

	// put the days (note -1) */
	current_time += (day - 1) * 60 * 60 * 24;

	// take the months - despite popular beliefs, leap years aren't just evenly divisible by 4 */
	if (((((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0)) && month > 2)
		current_time += (month_to_day_conversion[month - 1] + 1) * 60 * 60 * 24;
	else
		current_time += (month_to_day_conversion[month - 1]) * 60 * 60 * 24;

	// put the years
	int year_count = (year - 1949);

	for (int i = 0; i < year_count - 1; i++)
		current_time += (((((i+1950) % 4) == 0) && (((i+1950) % 100) != 0)) || (((i+1950) % 400) == 0)) ? 60*60*24*366 : 60*60*24*365;

	m_rtc_reg_lo = current_time & 0x0000ffff;
	m_rtc_reg_hi = (current_time & 0xffff0000) >> 16;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void aicartc_device::device_reset()
{
	m_rtc_tick = 0;
}


//-------------------------------------------------
//  clock_tick - advance the RTC counter
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(aicartc_device::clock_tick)
{
	m_rtc_tick++;
	if(m_rtc_tick & 0x8000)
	{
		m_rtc_tick = 0;
		m_rtc_reg_lo++;
		if(m_rtc_reg_lo == 0)
			m_rtc_reg_hi++;
	}
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

uint16_t aicartc_device::read(offs_t offset)
{
	uint16_t res;

	res = 0;
	switch(offset)
	{
		case 0:
			res = m_rtc_reg_hi; break;
		case 1:
			res = m_rtc_reg_lo; break;
	}

	return res;
}

void aicartc_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch(offset)
	{
		case 0:
			if(m_we)
			{
				COMBINE_DATA(&m_rtc_reg_hi);
				m_we = 0;
			}

			break;

		case 1:
			if(m_we)
			{
				COMBINE_DATA(&m_rtc_reg_lo);
				m_rtc_tick = 0; // low register also clears tick count
			}

			break;

		case 2:
			m_we = data & 1;
			break;
	}

}
