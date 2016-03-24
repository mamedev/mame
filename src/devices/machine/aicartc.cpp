// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    AICA-RTC sub-device

    TODO:
    - move this inside AICA sound core once that'll get modernized

***************************************************************************/

#include "emu.h"
#include "machine/aicartc.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type AICARTC = &device_creator<aicartc_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  aicartc_device - constructor
//-------------------------------------------------

aicartc_device::aicartc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, AICARTC, "AICA RTC", tag, owner, clock, "aicartc", __FILE__),
		device_rtc_interface(mconfig, *this), m_rtc_reg_lo(0), m_rtc_reg_hi(0), m_rtc_tick(0), m_we(0), m_clock_timer(nullptr)
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
	m_clock_timer = timer_alloc();
	m_clock_timer->adjust(attotime::from_hz(clock()), 0, attotime::from_hz(clock()));

	{
		UINT32 current_time;
		int year_count,cur_year,i;
		const int month_to_day_conversion[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
		system_time systime;
		machine().base_datetime(systime);

		/* put the seconds */
		current_time = systime.local_time.second;
		/* put the minutes */
		current_time+= systime.local_time.minute*60;
		/* put the hours */
		current_time+= systime.local_time.hour*60*60;
		/* put the days (note -1) */
		current_time+= (systime.local_time.mday-1)*60*60*24;
		/* take the current year here for calculating leaps */
		cur_year = (systime.local_time.year);

		/* take the months - despite popular beliefs, leap years aren't just evenly divisible by 4 */
		if(((((cur_year % 4) == 0) && ((cur_year % 100) != 0)) || ((cur_year % 400) == 0)) && systime.local_time.month > 2)
			current_time+= (month_to_day_conversion[systime.local_time.month]+1)*60*60*24;
		else
			current_time+= (month_to_day_conversion[systime.local_time.month])*60*60*24;

		/* put the years */
		year_count = (cur_year-1949);

		for(i=0;i<year_count-1;i++)
			current_time += (((((i+1950) % 4) == 0) && (((i+1950) % 100) != 0)) || (((i+1950) % 400) == 0)) ? 60*60*24*366 : 60*60*24*365;

		m_rtc_reg_lo = current_time & 0x0000ffff;
		m_rtc_reg_hi = (current_time & 0xffff0000) >> 16;
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void aicartc_device::device_reset()
{
	m_rtc_tick = 0;
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------


void aicartc_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
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

READ16_MEMBER( aicartc_device::read )
{
	UINT16 res;

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

WRITE16_MEMBER( aicartc_device::write )
{
	switch(offset)
	{
		case 0:
			if(m_we)
			{
				COMBINE_DATA(&m_rtc_reg_hi);
				// clear write enable here?
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
