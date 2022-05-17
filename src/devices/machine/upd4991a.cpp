// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

    NEC uPD4991/uPD4991a parallel RTC

    uPD4991 should be very similar but with "30% more power consumption" (cit.)

    TODO:
    - bare minimum to make PC98HA happy;
    - set clock regs;
    - alarm & timer pulse;
    - AM/PM hour mode;
    - leap year;
    - busy flag;
    - adjust/clock stop/clock wait mechanisms;

**************************************************************************************************/

#include "emu.h"
#include "machine/upd4991a.h"


//*************************************************************************************************
//  GLOBAL VARIABLES
//*************************************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(UPD4991A, upd4991a_device, "upd4991a", "NEC uPD4991a parallel RTC")


//*************************************************************************************************
//  LIVE DEVICE
//*************************************************************************************************

//-------------------------------------------------
//  upd4991a_device - constructor
//-------------------------------------------------

upd4991a_device::upd4991a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, UPD4991A, tag, owner, clock)
	, device_rtc_interface(mconfig, *this)
	, m_timer_clock(nullptr)
{
	std::fill(std::begin(m_rtc_regs), std::end(m_rtc_regs), 0);
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void upd4991a_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd4991a_device::device_start()
{
	m_timer_clock = timer_alloc(FUNC(upd4991a_device::clock_tick), this);
	m_timer_clock->adjust(attotime::from_hz(clock() / 32768), 0, attotime::from_hz(clock() / 32768));

	save_item(NAME(m_address));
}


TIMER_CALLBACK_MEMBER(upd4991a_device::clock_tick)
{
	advance_seconds();
}

//-------------------------------------------------
//  rtc_clock_updated -
//-------------------------------------------------

void upd4991a_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
/*
[0-1]
xxxx xxxx seconds
[2-3]
xxxx xxxx minutes
[4-5]
xxxx xxxx hour
[6-7]
---- xxxx date digit (weekday?)
xxxx ---- 1 day digit
[8-9]
---- xxxx 10 day digit
xxxx ---- 1 month digit
[a-b]
---- xxxx 10 month digit
xxxx ---- 1 year digit
[c-d]
---- xxxx 10 year digit
xxxx ---- control register 1 (write only)
[e-f]
---- xxxx control register 2 (read/write)
xxxx ---- mode register (write only)
*/
	m_rtc_regs[0] = convert_to_bcd(second);
	m_rtc_regs[1] = convert_to_bcd(minute);
	m_rtc_regs[2] = convert_to_bcd(hour);
	const u8 bcd_day = convert_to_bcd(day);
	const u8 bcd_month = convert_to_bcd(month);
	const u8 bcd_year = convert_to_bcd(year);
	m_rtc_regs[3] = (day_of_week-1) | ((bcd_day & 0x0f) << 4);
	m_rtc_regs[4] = ((bcd_day & 0xf0) >> 4) | ((bcd_month & 0x0f) << 4);
	m_rtc_regs[5] = ((bcd_month & 0xf0) >> 4) | ((bcd_year & 0x0f) << 4);
	m_rtc_regs[6] = ((bcd_year & 0xf0) >> 4);
}

//*************************************************************************************************
//  READ/WRITE HANDLERS
//*************************************************************************************************

u8 upd4991a_device::data_r(offs_t offset)
{
	return m_rtc_regs[m_address >> 1] >> ((m_address & 1) ? 4 : 0);
}

void upd4991a_device::data_w(offs_t offset, u8 data)
{
	// ...
}

void upd4991a_device::address_w(offs_t offset, u8 data)
{
	m_address = data & 0xf;
}
