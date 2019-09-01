// license:BSD-3-Clause
// copyright-holders:Raphael Nabet,Sergey Svishchev
/***************************************************************************

    mm58174.c

    National Semiconductor MM58174 Microprocessor Compatible Real Time Clock

	Docs:
	* <ftp://ftp.jameco.com/Archive/Obsolete-TechDocuments/26219.pdf>

    Todo:
    * interrupt pin output

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
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mm58174_device::device_start()
{
	m_increment_rtc = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mm58174_device::rtc_increment_cb),this));
	m_increment_rtc->adjust(attotime::zero, 0, attotime::from_msec(100));
	m_interrupt_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mm58174_device::rtc_interrupt_cb),this));

	// register for state saving
	save_item(NAME(m_status));
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
//  device_reset - device-specific reset
//-------------------------------------------------

void mm58174_device::device_reset()
{
	system_time systime;

	/* get the current date/time from the core */
	machine().current_datetime(systime);

	/* leap year indicator */
	m_years = 1 << (3 - (systime.local_time.year & 3));

	/* The clock count starts on 1st January 1900 */
	m_wday = 1 + ((systime.local_time.weekday) % 7);
	m_months1 = (systime.local_time.month + 1) / 10;
	m_months2 = (systime.local_time.month + 1) % 10;
	m_days1 = systime.local_time.mday / 10;
	m_days2 = systime.local_time.mday % 10;
	m_hours1 = systime.local_time.hour / 10;
	m_hours2 = systime.local_time.hour % 10;
	m_minutes1 = systime.local_time.minute / 10;
	m_minutes2 = systime.local_time.minute % 10;
	m_seconds1 = systime.local_time.second / 10;
	m_seconds2 = systime.local_time.second % 10;
	m_tenths = 0;
	m_status = 0;
	m_control = 0;
}


attotime mm58174_device::interrupt_period_table(int val)
{
	switch(val)
	{
		case 0: return attotime::from_msec(0);
		case 1: return attotime::from_msec(500);
		case 2: return attotime::from_seconds(5);
		case 4: return attotime::from_seconds(60);
		default: fatalerror("out of range\n");
	}
}

uint8_t mm58174_device::read(offs_t offset)
{
	int reply = 0;

	offset &= 0xf;

	switch (offset)
	{
		case 0x00:   /* Test Only */
			break;

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

		case 0x0d:   /* Units Years */
			break;

		case 0x0e:   /* Stop/Start */
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
			break;

		case 0x05:   /* Tens Minutes */
			m_minutes1 = data;
			break;

		case 0x06:   /* Units Hours */
			m_hours2 = data;
			break;

		case 0x07:   /* Tens Hours */
			m_hours1 = data;
			break;

		case 0x08:   /* Units Days */
			m_days2 = data;
			break;

		case 0x09:   /* Tens Days */
			m_days1 = data;
			break;

		case 0x0a:   /* Day of Week */
			m_wday = data;
			break;

		case 0x0b:   /* Units Months */
			m_months2 = data;
			break;

		case 0x0c:   /* Tens Months */
			m_months1 = data;
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


// Set RTC interrupt flag
TIMER_CALLBACK_MEMBER(mm58174_device::rtc_interrupt_cb)
{
}


// Increment RTC clock (timed interrupt every 1/10s)
TIMER_CALLBACK_MEMBER(mm58174_device::rtc_increment_cb)
{
	if (m_control & ctl_clkrun)
	{
		if ((++m_tenths) == 10)
		{
			m_tenths = 0;

			if ((++m_seconds2) == 10)
			{
				m_seconds2 = 0;

				if ((++m_seconds1) == 6)
				{
					m_seconds1 = 0;

					if ((++m_minutes2) == 10)
					{
						m_minutes2 = 0;

						if ((++m_minutes1) == 6)
						{
							m_minutes1 = 0;

							if ((++m_hours2) == 10)
							{
								m_hours2 = 0;

								m_hours1++;
							}

							/* handle wrap-around */
							if ((m_hours1*10 + m_hours2) == 24)
							{
								m_hours1 = m_hours2 = 0;
							}

							/* increment day if needed */
							if ((m_hours1*10 + m_hours2) == 0)
							{
								int days_in_month;

								if ((++m_days2) == 10)
								{
									m_days2 = 0;

									m_days1++;
								}

								if ((++m_wday) == 8)
									m_wday = 1;

								{
									static const int days_in_month_array[] =
									{
										31,28,31, 30,31,30,
										31,31,30, 31,30,31
									};

									if (((m_months1*10 + m_months2) != 2) || !(m_years & year_leap))
										days_in_month = days_in_month_array[m_months1*10 + m_months2 - 1];
									else
										days_in_month = 29;
								}


								if ((m_days1*10 + m_days2) == days_in_month+1)
								{
									m_days1 = 0;
									m_days2 = 1;

									if ((++m_months2) == 10)
									{
										m_months2 = 0;

										m_months1++;
									}

									if ((m_months1*10 + m_months2) == 13)
									{
										m_months1 = 0;
										m_months2 = 1;

										m_years <<= 1;
										m_years = (m_years & 15) | (m_years >> 4);
									}
								}
							}
						}
					}
				}
			}
		}
	}
}
