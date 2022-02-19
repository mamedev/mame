// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/***************************************************************************

    uPD4992 parallel RTC

    TODO:
    - Add timers
    - Add leap year count
    - Add 12 hours mode
    - Add mode/control register

***************************************************************************/

#include "emu.h"
#include "machine/upd4992.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(UPD4992, upd4992_device, "upd4992", "uPD4992 RTC")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  upd4992_device - constructor
//-------------------------------------------------

upd4992_device::upd4992_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, UPD4992, tag, owner, clock)
	, device_rtc_interface(mconfig, *this)
	, m_timer_clock(nullptr)
{
	std::fill(std::begin(m_rtc_regs), std::end(m_rtc_regs), 0);
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void upd4992_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd4992_device::device_start()
{
	m_timer_clock = timer_alloc(TIMER_CLOCK);
	m_timer_clock->adjust(attotime::from_hz(clock() / 32768), 0, attotime::from_hz(clock() / 32768));
}


void upd4992_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case TIMER_CLOCK:
		advance_seconds();
		break;
	}
}

//-------------------------------------------------
//  rtc_clock_updated -
//-------------------------------------------------

void upd4992_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
/*
[2]
x--- ---- 12/24H flag
-x-- ---- AM/PM flag
--xx ---- 10 hour digit
---- xxxx 1s hour digit
[3]
xx-- ---- Leap year control
--xx ---- Leap year counter
---- xxxx Day of week digit
[4]
xxxx ---- 10s day digit
---- xxxx 1s day digit
[5]
xxxx ---- 10s month digit
---- xxxx 1s month digit
[6]
xxxx ---- 10s year digit
---- xxxx 1s year digit
[7]
xxxx ---- Mode register
---- xxxx Control Register
*/
	m_rtc_regs[0] = convert_to_bcd(second);
	m_rtc_regs[1] = convert_to_bcd(minute);
	m_rtc_regs[2] = convert_to_bcd(hour);
	m_rtc_regs[3] = day_of_week-1;
	m_rtc_regs[4] = convert_to_bcd(day);
	m_rtc_regs[5] = convert_to_bcd(month);
	m_rtc_regs[6] = convert_to_bcd(year);
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

u8 upd4992_device::read(offs_t offset)
{
	return m_rtc_regs[offset];
}

void upd4992_device::write(offs_t offset, u8 data)
{
	if(offset == 7)
	{
		if(data & 8)
		{
			if(data & 2) // reset
			{
				// ...
			}

			m_timer_clock->enable(data & 1);
		}
	}
	else // TODO: perhaps there's a write inhibit?
	{
		m_rtc_regs[offset] = data;
		set_time(1, bcd_to_integer(m_rtc_regs[6]),
					bcd_to_integer(m_rtc_regs[5]),
					bcd_to_integer(m_rtc_regs[4]),
					m_rtc_regs[3]+1,
					bcd_to_integer(m_rtc_regs[2]),
					bcd_to_integer(m_rtc_regs[1]),
					bcd_to_integer(m_rtc_regs[0]));
	}
}
