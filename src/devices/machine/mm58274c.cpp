// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/***************************************************************************

    mm58274c.c

    mm58274c emulation

    Reference:
    * National Semiconductor MM58274C Microprocessor Compatible Real Time Clock
        <http://www.national.com/ds/MM/MM58274C.pdf>

    Todo:
    * Clock initialization will only work with the BwG: we need to provide
      a way to customize it.
    * Save the config to NVRAM?
    * Support interrupt pin output

    Raphael Nabet, 2002

***************************************************************************/

#include "emu.h"
#include "machine/mm58274c.h"

enum
{
	st_dcf = 0x8,       /* data-changed flag */
	st_if = 0x1,        /* interrupt flag */

	ctl_test = 0x8,     /* test mode (0=normal, 1=test) (not emulated) */
	ctl_clkstop = 0x4,  /* clock start/stop (0=run, 1=stop) */
	ctl_intsel = 0x2,   /* interrupt select (0=clock setting register, 1=interrupt register) */
	ctl_intstop = 0x1,  /* interrupt start stop (0=interrupt run, 1=interrupt stop) */

	clk_set_leap = 0xc,     /* leap year counter (0 indicates a leap year) */
	clk_set_leap_inc = 0x4, /* leap year increment */
	clk_set_pm = 0x2,       /* am/pm indicator (0 = am, 1 = pm, 0 in 24-hour mode) */
	clk_set_24 = 0x1,       /* 12/24-hour select bit (1= 24-hour mode) */

	int_ctl_rpt = 0x8,      /* 1 for repeated interrupt */
	int_ctl_dly = 0x7       /* 0 no interrupt, 1 = .1 second, 2=.5, 3=1, 4=5, 5=10, 6=30, 7=60 */
};



const device_type MM58274C = &device_creator<mm58274c_device>;


mm58274c_device::mm58274c_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
				: device_t(mconfig, MM58274C, "National Semiconductor MM58274C", tag, owner, clock, "mm58274c", __FILE__),
					m_mode24(0),
					m_day1(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mm58274c_device::device_start()
{
	m_increment_rtc = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mm58274c_device::rtc_increment_cb),this));
	m_increment_rtc->adjust(attotime::zero, 0, attotime::from_msec(100));
	m_interrupt_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mm58274c_device::rtc_interrupt_cb),this));

	// register for state saving
	save_item(NAME(m_status));
	save_item(NAME(m_control));
	save_item(NAME(m_clk_set));
	save_item(NAME(m_int_ctl));
	save_item(NAME(m_wday));
	save_item(NAME(m_years1));
	save_item(NAME(m_years2));
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

void mm58274c_device::device_reset()
{
	system_time systime;

	/* get the current date/time from the core */
	machine().current_datetime(systime);

	m_clk_set = systime.local_time.year & 3 << 2;
	if (m_mode24)
		m_clk_set |= clk_set_24;

	/* The clock count starts on 1st January 1900 */
	m_wday = 1 + ((systime.local_time.weekday - m_day1) % 7);
	m_years1 = (systime.local_time.year / 10) % 10;
	m_years2 = systime.local_time.year % 10;
	m_months1 = (systime.local_time.month + 1) / 10;
	m_months2 = (systime.local_time.month + 1) % 10;
	m_days1 = systime.local_time.mday / 10;
	m_days2 = systime.local_time.mday % 10;
	if (!m_mode24)
	{
		/* 12-hour mode */
		if (systime.local_time.hour > 12)
		{
			systime.local_time.hour -= 12;
			m_clk_set |= clk_set_pm;
		}
		if (systime.local_time.hour == 0)
			systime.local_time.hour = 12;
	}
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



attotime mm58274c_device::interrupt_period_table(int val)
{
	switch(val)
	{
		case 0: return attotime::from_msec(0);
		case 1: return attotime::from_msec(100);
		case 2: return attotime::from_msec(500);
		case 3: return attotime::from_seconds(1);
		case 4: return attotime::from_seconds(5);
		case 5: return attotime::from_seconds(10);
		case 6: return attotime::from_seconds(30);
		case 7: return attotime::from_seconds(60);
		default: fatalerror("out of range\n");
	}
}

READ8_MEMBER( mm58274c_device::read )
{
	int reply;

	offset &= 0xf;

	switch (offset)
	{
		case 0x00:   /* Control Register */
			reply = m_status;
			m_status = 0;
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

		case 0x0a:   /* Units Months */
			reply = m_months2;
			break;

		case 0x0b:   /* Tens Months */
			reply = m_months1;
			break;

		case 0x0c:   /* Units Years */
			reply = m_years2;
			break;

		case 0x0d:   /* Tens Years */
			reply = m_years1;
			break;

		case 0x0e:   /* Day of Week */
			reply = m_wday;
			break;

		case 0x0f:   /* Clock Setting & Interrupt Registers */
			if (m_control & ctl_intsel) /* interrupt register */
				reply = m_int_ctl;
			else    /* clock setting register */
			{
				if (m_clk_set & clk_set_24) /* 24-hour mode */
					reply = m_clk_set & ~clk_set_pm;
				else    /* 12-hour mode */
					reply = m_clk_set;
			}
			break;

		default:
			reply = 0;
			break;
	}

	return reply;
}


WRITE8_MEMBER( mm58274c_device::write )
{
	offset &= 0xf;
	data &= 0xf;

	switch (offset)
	{
		case 0x00:   /* Control Register (test mode and interrupt not emulated) */
			if ((!(m_control & ctl_intstop)) && (data & ctl_intstop))   /* interrupt stop */
				m_interrupt_timer->enable(0);
			else if ((m_control & ctl_intstop) && (!(data & ctl_intstop)))  /* interrupt run */
			{
				attotime period = interrupt_period_table(m_int_ctl & int_ctl_dly);

				m_interrupt_timer->adjust(period, 0, m_int_ctl & int_ctl_rpt ? period : attotime::zero);
			}
			if (data & ctl_clkstop) /* stopping the clock clears the tenth counter */
				m_tenths = 0;
			m_control = data;
			break;

		case 0x01:   /* Tenths of Seconds: cannot be written */
			break;

		case 0x02:   /* Units Seconds */
			m_seconds2 = data;
			break;

		case 0x03:   /* Tens Seconds */
			m_seconds1 = data;
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

		case 0x0a:   /* Units Months */
			m_months2 = data;
			break;

		case 0x0b:   /* Tens Months */
			m_months1 = data;
			break;

		case 0x0c:   /* Units Years */
			m_years2 = data;
			break;

		case 0x0d:   /* Tens Years */
			m_years1 = data;
			break;

		case 0x0e:   /* Day of Week */
			m_wday = data;
			break;

		case 0x0f:   /* Clock Setting & Interrupt Registers */
			if (m_control & ctl_intsel) /* interrupt register (not emulated) */
			{
				m_int_ctl = data;
				if (!(m_control & ctl_intstop)) /* interrupt run */
				{
					attotime period = interrupt_period_table(m_int_ctl & int_ctl_dly);

					m_interrupt_timer->adjust(period, 0, m_int_ctl & int_ctl_rpt ? period : attotime::zero);
				}
			}
			else    /* clock setting register */
			{
				m_clk_set = data;
#if 0
				if (m_clk_set & clk_set_24) /* 24-hour mode */
					m_clk_set &= ~clk_set_pm;
#endif
			}
			break;
	}
}


// Set RTC interrupt flag
TIMER_CALLBACK_MEMBER(mm58274c_device::rtc_interrupt_cb)
{
	m_status |= st_if;
}


// Increment RTC clock (timed interrupt every 1/10s)
TIMER_CALLBACK_MEMBER(mm58274c_device::rtc_increment_cb)
{
	if (!(m_control & ctl_clkstop))
	{
		m_status |= st_dcf;

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
							if ((!(m_clk_set & clk_set_24))
									&& ((m_hours1*10 + m_hours2) == 12))
							{
								m_clk_set ^= clk_set_pm;
							}
							if ((!(m_clk_set & clk_set_24))
									&& ((m_hours1*10 + m_hours2) == 13))
							{
								m_hours1 = 0;
								m_hours2 = 1;
							}

							if ((m_clk_set & clk_set_24)
								&& ((m_hours1*10 + m_hours2) == 24))
							{
								m_hours1 = m_hours2 = 0;
							}

							/* increment day if needed */
							if ((m_clk_set & clk_set_24)
								? ((m_hours1*10 + m_hours2) == 0)
								: (((m_hours1*10 + m_hours2) == 12)
									&& (!(m_clk_set & clk_set_pm))))
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

									if (((m_months1*10 + m_months2) != 2) || (m_clk_set & clk_set_leap))
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

										m_clk_set = (m_clk_set & ~clk_set_leap)
															| ((m_clk_set + clk_set_leap_inc) & clk_set_leap);

										if ((++m_years2) == 10)
										{
											m_years2 = 0;

											if ((++m_years1) == 10)
												m_years1 = 0;
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
}
