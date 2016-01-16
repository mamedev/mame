// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    v3021.c

    EM Microelectronic-Marin SA Ultra Low Power 32kHz CMOS RTC (DIP8)

    Serial Real Time Clock

    - very preliminary, borrowed from hard-coded PGM implementation.

***************************************************************************/

#include "emu.h"
#include "machine/v3021.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type v3021 = &device_creator<v3021_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  v3021_device - constructor
//-------------------------------------------------

v3021_device::v3021_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, v3021, "V3021 RTC", tag, owner, clock, "v3021", __FILE__), m_cal_mask(0), m_cal_com(0), m_cal_cnt(0), m_cal_val(0)
{
}

void v3021_device::timer_callback()
{
	static const UINT8 dpm[12] = { 0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31 };
	int dpm_count;

	m_rtc.sec++;

	if((m_rtc.sec & 0x0f) >= 0x0a)              { m_rtc.sec+=0x10; m_rtc.sec&=0xf0; }
	if((m_rtc.sec & 0xf0) >= 0x60)              { m_rtc.min++; m_rtc.sec = 0; }
	if((m_rtc.min & 0x0f) >= 0x0a)              { m_rtc.min+=0x10; m_rtc.min&=0xf0; }
	if((m_rtc.min & 0xf0) >= 0x60)              { m_rtc.hour++; m_rtc.min = 0; }
	if((m_rtc.hour & 0x0f) >= 0x0a)             { m_rtc.hour+=0x10; m_rtc.hour&=0xf0; }
	if((m_rtc.hour & 0xff) >= 0x24)             { m_rtc.day++; m_rtc.wday<<=1; m_rtc.hour = 0; }
	if(m_rtc.wday & 0x80)                       { m_rtc.wday = 1; }
	if((m_rtc.day & 0x0f) >= 0x0a)              { m_rtc.day+=0x10; m_rtc.day&=0xf0; }

	/* TODO: crude leap year support */
	dpm_count = (m_rtc.month & 0xf) + (((m_rtc.month & 0x10) >> 4)*10)-1;

	if(((m_rtc.year % 4) == 0) && m_rtc.month == 2)
	{
		if((m_rtc.day & 0xff) >= dpm[dpm_count]+1+1)
			{ m_rtc.month++; m_rtc.day = 0x01; }
	}
	else if((m_rtc.day & 0xff) >= dpm[dpm_count]+1){ m_rtc.month++; m_rtc.day = 0x01; }
	if((m_rtc.month & 0x0f) >= 0x0a)            { m_rtc.month = 0x10; }
	if(m_rtc.month >= 0x13)                     { m_rtc.year++; m_rtc.month = 1; }
	if((m_rtc.year & 0x0f) >= 0x0a)             { m_rtc.year+=0x10; m_rtc.year&=0xf0; }
	if((m_rtc.year & 0xf0) >= 0xa0)             { m_rtc.year = 0; } //2000-2099 possible timeframe
}

TIMER_CALLBACK( v3021_device::rtc_inc_callback )
{
	reinterpret_cast<v3021_device *>(ptr)->timer_callback();
}

//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void v3021_device::device_validity_check(validity_checker &valid) const
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void v3021_device::device_start()
{
	/* let's call the timer callback every second */
	machine().scheduler().timer_pulse(attotime::from_hz(clock() / XTAL_32_768kHz), FUNC(rtc_inc_callback), 0, (void *)this);

	system_time systime;
	machine().base_datetime(systime);

	m_rtc.day = ((systime.local_time.mday / 10)<<4) | ((systime.local_time.mday % 10) & 0xf);
	m_rtc.month = (((systime.local_time.month+1) / 10) << 4) | (((systime.local_time.month+1) % 10) & 0xf);
	m_rtc.wday = 1 << systime.local_time.weekday;
	m_rtc.year = (((systime.local_time.year % 100)/10)<<4) | ((systime.local_time.year % 10) & 0xf);
	m_rtc.hour = ((systime.local_time.hour / 10)<<4) | ((systime.local_time.hour % 10) & 0xf);
	m_rtc.min = ((systime.local_time.minute / 10)<<4) | ((systime.local_time.minute % 10) & 0xf);
	m_rtc.sec = ((systime.local_time.second / 10)<<4) | ((systime.local_time.second % 10) & 0xf);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void v3021_device::device_reset()
{
	m_cal_cnt = 0;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ8_MEMBER( v3021_device::read )
{
	UINT8 calr = (m_cal_val & m_cal_mask) ? 1 : 0;

	m_cal_mask <<= 1;
	return calr;
}

WRITE8_MEMBER( v3021_device::write )
{
	m_cal_com <<= 1;
	m_cal_com |= data & 1;
	++m_cal_cnt;

	if (m_cal_cnt == 4)
	{
		m_cal_mask = 1;
		m_cal_val = 1;
		m_cal_cnt = 0;

		switch (m_cal_com & 0xf)
		{
			case 1: case 3: case 5: case 7: case 9: case 0xb: case 0xd:
				m_cal_val++;
				break;

			case 0:
				m_cal_val = (m_rtc.wday); //??
				break;

			case 2:  //Hours
				m_cal_val = (m_rtc.hour);
				break;

			case 4:  //Seconds
				m_cal_val = (m_rtc.sec);
				break;

			case 6:  //Month
				m_cal_val = (m_rtc.month); //?? not bcd in MVS
				break;

			case 8:
				m_cal_val = 0; //Controls blinking speed, maybe milliseconds
				break;

			case 0xa: //Day
				m_cal_val = (m_rtc.day);
				break;

			case 0xc: //Minute
				m_cal_val = (m_rtc.min);
				break;

			case 0xe:  //Year
				m_cal_val = (m_rtc.year % 100);
				break;

			case 0xf:  //Load Date
				//space.machine().base_datetime(m_systime);
				break;
		}
	}
}
