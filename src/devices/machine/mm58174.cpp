// license:BSD-3-Clause
// copyright-holders:Raphael Nabet,Sergey Svishchev
/***************************************************************************

    mm58174.cpp

    National Semiconductor MM58174 Microprocessor Compatible Real Time Clock

    Docs:
    * <http://www.bitsavers.org/components/national/_appNotes/AN-0359.pdf>
    * <ftp://ftp.jameco.com/Archive/Obsolete-TechDocuments/26219.pdf>

    Todo:
    * data-changed flip-flop
    * interrupts, MM58174A (no interrupt ack)
    * loss of accuracy after restart

***************************************************************************/

#include "emu.h"
#include "machine/mm58174.h"

enum
{
	ctl_clkrun = 0x1,  /* clock start/stop (1=run, 0=stop) */

	year_leap = 0x8,

	int_ctl_rpt = 0x8,  /* 1 for repeated interrupt */
	int_ctl_dly = 0x7   /* 0 no interrupt, 1 = .5 second, 2=5, 4=60 */
};


DEFINE_DEVICE_TYPE(MM58174, mm58174_device, "mm58174", "National Semiconductor MM58174 RTC")


mm58174_device::mm58174_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MM58174, tag, owner, clock)
	, device_rtc_interface(mconfig, *this)
	, m_control(0)
	, m_int_ctl(0)
	, m_tenths(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mm58174_device::device_start()
{
	m_rtc_timer = timer_alloc(FUNC(mm58174_device::clock_tick), this);
	m_rtc_timer->adjust(attotime::zero, 0, attotime::from_msec(100));
	m_interrupt_timer = timer_alloc(FUNC(mm58174_device::scheduler_sync), this);

	// register for state saving
	save_item(NAME(m_control));
	save_item(NAME(m_int_ctl));
	save_item(NAME(m_wday));
	save_item(NAME(m_months1));
	save_item(NAME(m_months2));
	save_item(NAME(m_days1));
	save_item(NAME(m_days2));
	save_item(NAME(m_hours1));
	save_item(NAME(m_hours2));
	save_item(NAME(m_minutes1));
	save_item(NAME(m_minutes2));
	save_item(NAME(m_seconds1));
	save_item(NAME(m_seconds2));
	save_item(NAME(m_tenths));
}

//-------------------------------------------------
//  rtc_clock_updated -
//-------------------------------------------------

void mm58174_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_seconds1 = second / 10;
	m_seconds2 = second % 10;
	m_minutes1 = minute / 10;
	m_minutes2 = minute % 10;
	m_hours1 = hour / 10;
	m_hours2 = hour % 10;
	m_wday = day_of_week;
	m_days1 = day / 10;
	m_days2 = day % 10;
	m_months1 = month / 10;
	m_months2 = month % 10;
	m_years = 1 << (3 - (year & 3));
}


attotime mm58174_device::interrupt_period_table(int val)
{
	switch(val)
	{
		case 0: return attotime::never;
		case 1: return attotime::from_msec(500);
		case 2: return attotime::from_seconds(5);
		case 4: return attotime::from_seconds(60);
		default: fatalerror("out of range\n");
	}
}

void mm58174_device::update_rtc()
{
	set_clock_register(RTC_SECOND, m_seconds1 * 10 + m_seconds2);
	set_clock_register(RTC_MINUTE, m_minutes1 * 10 + m_minutes2);
	set_clock_register(RTC_HOUR, m_hours1 * 10 + m_hours2);
	set_clock_register(RTC_DAY, m_days1 * 10 + m_days2);
	set_clock_register(RTC_DAY_OF_WEEK, m_wday);
	set_clock_register(RTC_MONTH, m_months1 * 10 + m_months2);
}

uint8_t mm58174_device::read(offs_t offset)
{
	int reply = 0;

	offset &= 0xf;

	switch (offset)
	{
		case 0x01:   /* Tenths of Seconds */
			reply = m_tenths;
			break;

		case 0x02:   /* Units Seconds */
			reply = m_seconds2;
			break;

		case 0x03:   /* Tens Seconds */
			reply = m_seconds1;
			break;

		case 0x04:  /* Units Minutes */
			reply = m_minutes2;
			break;

		case 0x05:   /* Tens Minutes */
			reply = m_minutes1;
			break;

		case 0x06:   /* Units Hours */
			reply = m_hours2;
			break;

		case 0x07:   /* Tens Hours */
			reply = m_hours1;
			break;

		case 0x08:   /* Units Days */
			reply = m_days2;
			break;

		case 0x09:   /* Tens Days */
			reply = m_days1;
			break;

		case 0x0a:   /* Day of Week */
			reply = m_wday;
			break;

		case 0x0b:   /* Units Months */
			reply = m_months2;
			break;

		case 0x0c:   /* Tens Months */
			reply = m_months1;
			break;

		case 0x0f:   /* Clock Setting & Interrupt Registers */
			reply = m_int_ctl;
			break;

		default:
			reply = 0;
			break;
	}

	logerror("reg %02x == %02x\n", offset, reply);

	return reply;
}


void mm58174_device::write(offs_t offset, uint8_t data)
{
	offset &= 0xf;
	data &= 0xf;

	logerror("reg %02x <- %02x\n", offset, data);

	switch (offset)
	{
		case 0x00:   /* Test Mode Register (emulated) */
			break;

		case 0x01:   /* Tenths of Seconds: cannot be written */
			break;

		case 0x02:   /* Units Seconds: cannot be written */
			break;

		case 0x03:   /* Tens Seconds: cannot be written */
			break;

		case 0x04:   /* Units Minutes */
			m_minutes2 = data;
			update_rtc();
			break;

		case 0x05:   /* Tens Minutes */
			m_minutes1 = data;
			update_rtc();
			break;

		case 0x06:   /* Units Hours */
			m_hours2 = data;
			update_rtc();
			break;

		case 0x07:   /* Tens Hours */
			m_hours1 = data;
			update_rtc();
			break;

		case 0x08:   /* Units Days */
			m_days2 = data;
			update_rtc();
			break;

		case 0x09:   /* Tens Days */
			m_days1 = data;
			update_rtc();
			break;

		case 0x0a:   /* Day of Week */
			m_wday = data;
			update_rtc();
			break;

		case 0x0b:   /* Units Months */
			m_months2 = data;
			update_rtc();
			break;

		case 0x0c:   /* Tens Months */
			m_months1 = data;
			update_rtc();
			break;

		case 0x0d:   /* Years Status */
			break;

		case 0x0e:   /* Stop/Start */
			if ((m_control & ctl_clkrun) && (!(data & ctl_clkrun)))  /* interrupt stop */
				m_interrupt_timer->enable(0);
			else if ((!(m_control & ctl_clkrun)) && (data & ctl_clkrun))   /* interrupt run */
			{
				attotime period = interrupt_period_table(m_int_ctl & int_ctl_dly);

				m_interrupt_timer->adjust(period, 0, m_int_ctl & int_ctl_rpt ? period : attotime::zero);
			}
			if (!(data & ctl_clkrun)) /* stopping the clock clears the tenth counter */
				m_tenths = 0;
			m_control = data;
			break;

		case 0x0f:   /* Interrupt Register */
			m_int_ctl = data;
			if (m_control & ctl_clkrun) /* interrupt run */
			{
				attotime period = interrupt_period_table(m_int_ctl & int_ctl_dly);

				m_interrupt_timer->adjust(period, 0, m_int_ctl & int_ctl_rpt ? period : attotime::zero);
			}
			break;
	}
}


// Increment RTC clock (timed interrupt every 1/10s)
TIMER_CALLBACK_MEMBER(mm58174_device::clock_tick)
{
	if (m_control & ctl_clkrun)
	{
		if ((++m_tenths) == 10)
		{
			m_tenths = 0;
			advance_seconds();
		}
	}
}

// TODO: Investigate if this is necessary, or if there's missing functionality from the device.
// Prior to the device_timer removal, m_interrupt_timer was being allocated with the same default ID as m_rtc_timer.
// As there was no interrupt-related logic in the existing device_timer implementation, the only possible thing that
// m_interrupt_timer could possibly do, functionally, is force a scheduler sync by elapsing, and also cause the
// RTC to tick at an erroneous rate.
TIMER_CALLBACK_MEMBER(mm58174_device::scheduler_sync)
{
}
