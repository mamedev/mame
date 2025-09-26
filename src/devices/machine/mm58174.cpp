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
#include "mm58174.h"

#define LOG_PORT (1U << 1)
#define LOG_ERR  (1U << 2)

#define VERBOSE (0)

#include "logmacro.h"

#define LOGPORT(...)      LOGMASKED(LOG_PORT, __VA_ARGS__)
#define LOGERR(...)       LOGMASKED(LOG_ERR, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

namespace {

enum
{
	CTL_CLKRUN = 0x1,     // clock start/stop (1=run, 0=stop)

	INT_CTL_REPEAT = 0x8, // 1 for repeated interrupt
	INT_CTRL_DELAY = 0x7  // 0 no interrupt, 1 = .5 second, 2=5, 4=60 seconds
};

enum regs: u8
{
	TEST_ONLY = 0x00,         // Write Only
	TENTHS_OF_SECONDS = 0x01, // Read Only
	UNITS_OF_SECONDS = 0x02,  // Read Only
	TENS_OF_SECONDS = 0x03,   // Read Only
	UNITS_OF_MINUTES = 0x04,  // Read/Write
	TENS_OF_MINUTES = 0x05,   // Read/Write
	UNITS_OF_HOURS = 0x06,    // Read/Write
	TENS_OF_HOURS = 0x07,     // Read/Write
	UNITS_OF_DAYS = 0x08,     // Read/Write
	TENS_OF_DAYS = 0x09,      // Read/Write
	DAY_OF_WEEK = 0x0a,       // Read/Write
	UNITS_OF_MONTHS = 0x0b,   // Read/Write
	TENS_OF_MONTHS = 0x0c,    // Read/Write
	LEAP_YEAR_STATUS = 0x0d,  // Write Only
	STOP_START = 0x0e,        // Write Only
	INTERRUPT_REGISTER = 0x0f // Read/Write
};

} // anonymous namespace

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
	save_item(NAME(m_years));
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
	m_years = year & 3;  // only care about leap year status
}


attotime mm58174_device::interrupt_period_table(int val)
{
	switch (val)
	{
		case 0:
			return attotime::never;
		case 1:
			return attotime::from_msec(500);
		case 2:
			return attotime::from_seconds(5);
		case 4:
			return attotime::from_seconds(60);
		default:
			LOGERR("%s: out of range\n", FUNCNAME);
			return attotime::never;
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
	set_clock_register(RTC_YEAR, m_years);
}

uint8_t mm58174_device::read(offs_t offset)
{
	int reply = 0;

	offset &= 0xf;

	switch (offset)
	{
		case TENTHS_OF_SECONDS:
			reply = m_tenths;
			break;

		case UNITS_OF_SECONDS:
			reply = m_seconds2;
			break;

		case TENS_OF_SECONDS:
			reply = m_seconds1;
			break;

		case UNITS_OF_MINUTES:
			reply = m_minutes2;
			break;

		case TENS_OF_MINUTES:
			reply = m_minutes1;
			break;

		case UNITS_OF_HOURS:
			reply = m_hours2;
			break;

		case TENS_OF_HOURS:
			reply = m_hours1;
			break;

		case UNITS_OF_DAYS:
			reply = m_days2;
			break;

		case TENS_OF_DAYS:
			reply = m_days1;
			break;

		case DAY_OF_WEEK:
			reply = m_wday;
			break;

		case UNITS_OF_MONTHS:
			reply = m_months2;
			break;

		case TENS_OF_MONTHS:
			reply = m_months1;
			break;

		case INTERRUPT_REGISTER:
			reply = m_int_ctl;
			break;

		default:
			LOGERR("%s: address out of range: 0x%02x\n", FUNCNAME, offset);
			break;
	}

	LOGPORT("%s: reg %02x == %02x\n", FUNCNAME, offset, reply);
	return reply;
}


void mm58174_device::write(offs_t offset, uint8_t data)
{
	offset &= 0xf;
	data &= 0xf;

	LOGPORT("%s: reg %02x <- %02x\n", FUNCNAME, offset, data);

	switch (offset)
	{
		case TEST_ONLY:
		case TENTHS_OF_SECONDS:
		case UNITS_OF_SECONDS:
		case TENS_OF_SECONDS:
			LOGERR("%s: address out of range: 0x%02x\n", FUNCNAME, offset);
			break;

		case UNITS_OF_MINUTES:
			m_minutes2 = data;
			update_rtc();
			break;

		case TENS_OF_MINUTES:
			m_minutes1 = data & 0x7;
			update_rtc();
			break;

		case UNITS_OF_HOURS:
			m_hours2 = data;
			update_rtc();
			break;

		case TENS_OF_HOURS:
			m_hours1 = data & 0x3;
			update_rtc();
			break;

		case UNITS_OF_DAYS:
			m_days2 = data;
			update_rtc();
			break;

		case TENS_OF_DAYS:
			m_days1 = data & 0x3;
			update_rtc();
			break;

		case DAY_OF_WEEK:
			m_wday = data & 0x7;
			update_rtc();
			break;

		case UNITS_OF_MONTHS:
			m_months2 = data;
			update_rtc();
			break;

		case TENS_OF_MONTHS:
			m_months1 = data & 0x1;
			update_rtc();
			break;

		case LEAP_YEAR_STATUS:
			// Map to a value that works for the RTC implementation
			{
				int year = 0;
				if (BIT(data, 3))
				{
					year = 0;
				}
				else if (BIT(data, 2))
				{
					year = 3;
				}
				else if (BIT(data, 1))
				{
					year = 2;
				}
				else if (BIT(data, 0))
				{
					year = 1;
				}
				m_years = year;
				update_rtc();
			}
			break;

		case STOP_START:
			if ((m_control & CTL_CLKRUN) && (!(data & CTL_CLKRUN)))  // interrupt stop
				m_interrupt_timer->enable(0);
			else if ((!(m_control & CTL_CLKRUN)) && (data & CTL_CLKRUN))   // interrupt run
			{
				attotime period = interrupt_period_table(m_int_ctl & INT_CTRL_DELAY);

				m_interrupt_timer->adjust(period, 0, m_int_ctl & INT_CTL_REPEAT ? period : attotime::zero);
			}
			if (!(data & CTL_CLKRUN)) // stopping the clock clears the tenth counter
				m_tenths = 0;
			m_control = data;
			break;

		case INTERRUPT_REGISTER:
			m_int_ctl = data;
			if (m_control & CTL_CLKRUN) // interrupt run
			{
				attotime period = interrupt_period_table(m_int_ctl & INT_CTRL_DELAY);

				m_interrupt_timer->adjust(period, 0, m_int_ctl & INT_CTL_REPEAT ? period : attotime::zero);
			}
			break;
	}
}


// Increment RTC clock (timed interrupt every 1/10s)
TIMER_CALLBACK_MEMBER(mm58174_device::clock_tick)
{
	if (m_control & CTL_CLKRUN)
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
