// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/***************************************************************************

    uPD4992 parallel RTC

    References: NEC uPD4992 preliminary data sheet (document ID-3084) and
    uPD4992 user's manual (document S11812EJ4V0UM00).

    TODO:
    - Add timers
    - Add 12 hours mode
    - Add mode outputs, clock reset/adjust and BUSY/OSC status

***************************************************************************/

#include "emu.h"
#include "upd4992.h"


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
	, m_clock_remaining(attotime::zero)
	, m_time_initialized(false)
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
	save_item(NAME(m_rtc_regs));
	save_item(NAME(m_clock_remaining));
	save_item(NAME(m_time_initialized));

	m_timer_clock = timer_alloc(FUNC(upd4992_device::clock_tick), this);
	m_timer_clock->adjust(attotime::from_hz(clock() / 32768), 0, attotime::from_hz(clock() / 32768));
}


TIMER_CALLBACK_MEMBER(upd4992_device::clock_tick)
{
	if ((m_register[RTC_SECOND] == 59) && (m_register[RTC_MINUTE] == 59) && (m_register[RTC_HOUR] == 23))
	{
		// This part uses a separately programmable two-bit leap-year counter
		// rather than deriving leap years from the year register: February
		// has 29 days when leap years are enabled and the counter is 0
		// (data sheet, LEAP YEAR CONTROL REGISTER, p. 7).
		if ((m_register[RTC_MONTH] == 2) && (m_register[RTC_DAY] == 28) && !(m_rtc_regs[3] & 0xb0))
		{
			set_time(true, m_register[RTC_YEAR], 2, 29, (m_register[RTC_DAY_OF_WEEK] % 7) + 1, 0, 0, 0);
			return;
		}
		if ((m_register[RTC_MONTH] == 12) && (m_register[RTC_DAY] == 31))
			m_rtc_regs[3] = (m_rtc_regs[3] & 0xcf) | ((m_rtc_regs[3] + 0x10) & 0x30);
	}
	advance_seconds();
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
	if (!m_time_initialized)
	{
		m_rtc_regs[3] = (year & 3) << 4;
		m_time_initialized = true;
	}
	m_rtc_regs[0] = convert_to_bcd(second);
	m_rtc_regs[1] = convert_to_bcd(minute);
	m_rtc_regs[2] = convert_to_bcd(hour);
	m_rtc_regs[3] = (m_rtc_regs[3] & 0xf0) | ((day_of_week - 1) & 0x07);
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
	if (offset == 7)
	{
		m_rtc_regs[7] = data & 0xf0;
		// Control register (data sheet, CONTROL REGISTER, p. 8): with bit 3
		// clear, bit 0 is CLK stop (1 stops the clock, 0 starts it), bit 1
		// is reset and bit 2 is CLK adjust.  With bit 3 set the same bits
		// are INT stop, INT reset and TP enable, which do not affect the
		// time counter.  Software stops the clock before writing time data
		// (user's manual, section 3.1).
		if (!BIT(data, 3))
		{
			if (BIT(data, 0))
			{
				if (m_timer_clock->enabled())
					m_clock_remaining = m_timer_clock->remaining();
				m_timer_clock->enable(false);
			}
			else if (!m_timer_clock->enabled())
			{
				m_timer_clock->adjust(m_clock_remaining, 0, attotime::from_hz(clock() / 32768));
			}
		}
	}
	else // TODO: perhaps there's a write inhibit?
	{
		// Bit 6 enables writes to the leap-year counter; weekday writes without
		// it leave the counter unchanged.  Bit 7 disables leap-day insertion
		// (data sheet, LEAP YEAR CONTROL REGISTER, p. 7).
		if ((offset == 3) && !BIT(data, 6))
			m_rtc_regs[3] = (data & 0xcf) | (m_rtc_regs[3] & 0x30);
		else
			m_rtc_regs[offset] = data;
		set_time(1, bcd_to_integer(m_rtc_regs[6]),
					bcd_to_integer(m_rtc_regs[5]),
					bcd_to_integer(m_rtc_regs[4]),
					(m_rtc_regs[3] & 0x07) + 1,
					bcd_to_integer(m_rtc_regs[2]),
					bcd_to_integer(m_rtc_regs[1]),
					bcd_to_integer(m_rtc_regs[0]));
	}
}
