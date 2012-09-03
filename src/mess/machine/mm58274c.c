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
#include "mm58274c.h"

typedef struct _mm58274c_t mm58274c_t;

struct _mm58274c_t
{
	const mm58274c_interface *intf;

	int status;		/* status register (*read* from address 0 = control register) */
	int control;	/* control register (*write* to address 0) */

	int clk_set;	/* clock setting register */
	int int_ctl;	/* interrupt control register */


	int wday;		/* day of the week (1-7 (1=day1 as set in init)) */
	int years1;		/* years (BCD: 0-99) */
	int years2;
	int months1;	/* months (BCD: 1-12) */
	int months2;
	int days1;		/* days (BCD: 1-31) */
	int days2;
	int hours1;		/* hours (BCD : 0-23) */
	int hours2;
	int minutes1;	/* minutes (BCD : 0-59) */
	int minutes2;
	int seconds1;	/* seconds (BCD : 0-59) */
	int seconds2;
	int tenths;		/* tenths of second (BCD : 0-9) */

	emu_timer *increment_rtc;
	emu_timer *interrupt_timer;
};

enum
{
	st_dcf = 0x8,		/* data-changed flag */
	st_if = 0x1,		/* interrupt flag */

	ctl_test = 0x8,		/* test mode (0=normal, 1=test) (not emulated) */
	ctl_clkstop = 0x4,	/* clock start/stop (0=run, 1=stop) */
	ctl_intsel = 0x2,	/* interrupt select (0=clock setting register, 1=interrupt register) */
	ctl_intstop = 0x1,	/* interrupt start stop (0=interrupt run, 1=interrupt stop) */

	clk_set_leap = 0xc,		/* leap year counter (0 indicates a leap year) */
	clk_set_leap_inc = 0x4,	/* leap year increment */
	clk_set_pm = 0x2,		/* am/pm indicator (0 = am, 1 = pm, 0 in 24-hour mode) */
	clk_set_24 = 0x1,		/* 12/24-hour select bit (1= 24-hour mode) */

	int_ctl_rpt = 0x8,		/* 1 for repeated interrupt */
	int_ctl_dly = 0x7		/* 0 no interrupt, 1 = .1 second, 2=.5, 3=1, 4=5, 5=10, 6=30, 7=60 */
};


INLINE mm58274c_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == MM58274C);

	return (mm58274c_t *)downcast<mm58274c_device *>(device)->token();
}

static attotime interrupt_period_table(int val)
{
	switch(val)
	{
		case 0:	return attotime::from_msec(0);
		case 1:	return attotime::from_msec(100);
		case 2:	return attotime::from_msec(500);
		case 3:	return attotime::from_seconds(1);
		case 4:	return attotime::from_seconds(5);
		case 5:	return attotime::from_seconds(10);
		case 6:	return attotime::from_seconds(30);
		case 7:	return attotime::from_seconds(60);
		default: fatalerror("out of range");
	}
};

READ8_DEVICE_HANDLER( mm58274c_r )
{
	mm58274c_t *mm58274c = get_safe_token(device);
	int reply;

	offset &= 0xf;

	switch (offset)
	{
	case 0x0:	/* Control Register */
		reply = mm58274c->status;
		mm58274c->status = 0;
		break;

	case 0x1:	/* Tenths of Seconds */
		reply = mm58274c->tenths;
		break;

	case 0x2:	/* Units Seconds */
		reply = mm58274c->seconds2;
		break;

	case 0x3:	/* Tens Seconds */
		reply = mm58274c->seconds1;
		break;

	case 0x04:	/* Units Minutes */
		reply = mm58274c->minutes2;
		break;

	case 0x5:	/* Tens Minutes */
		reply = mm58274c->minutes1;
		break;

	case 0x6:	/* Units Hours */
		reply = mm58274c->hours2;
		break;

	case 0x7:	/* Tens Hours */
		reply = mm58274c->hours1;
		break;

	case 0x8:	/* Units Days */
		reply = mm58274c->days2;
		break;

	case 0x9:	/* Tens Days */
		reply = mm58274c->days1;
		break;

	case 0xA:	/* Units Months */
		reply = mm58274c->months2;
		break;

	case 0xB:	/* Tens Months */
		reply = mm58274c->months1;
		break;

	case 0xC:	/* Units Years */
		reply = mm58274c->years2;
		break;

	case 0xD:	/* Tens Years */
		reply = mm58274c->years1;
		break;

	case 0xE:	/* Day of Week */
		reply = mm58274c->wday;
		break;

	case 0xF:	/* Clock Setting & Interrupt Registers */
		if (mm58274c->control & ctl_intsel)
			/* interrupt register */
			reply = mm58274c->int_ctl;
		else
		{	/* clock setting register */
			if (mm58274c->clk_set & clk_set_24)
				/* 24-hour mode */
				reply = mm58274c->clk_set & ~clk_set_pm;
			else
				/* 12-hour mode */
				reply = mm58274c->clk_set;
		}
		break;

	default:
		reply = 0;
		break;
	}

	return reply;
}


WRITE8_DEVICE_HANDLER (mm58274c_w)
{
	mm58274c_t *mm58274c = get_safe_token(device);

	offset &= 0xf;
	data &= 0xf;

	switch (offset)
	{
	case 0x0:	/* Control Register (test mode and interrupt not emulated) */
		if ((! (mm58274c->control & ctl_intstop)) && (data & ctl_intstop))
			/* interrupt stop */
			mm58274c->interrupt_timer->enable(0);
		else if ((mm58274c->control & ctl_intstop) && (! (data & ctl_intstop)))
		{
			/* interrupt run */
			attotime period = interrupt_period_table(mm58274c->int_ctl & int_ctl_dly);

			mm58274c->interrupt_timer->adjust(period, 0, mm58274c->int_ctl & int_ctl_rpt ? period : attotime::zero);
		}
		if (data & ctl_clkstop)
			/* stopping the clock clears the tenth counter */
			mm58274c->tenths = 0;
		mm58274c->control = data;
		break;

	case 0x1:	/* Tenths of Seconds: cannot be written */
		break;

	case 0x2:	/* Units Seconds */
		mm58274c->seconds2 = data;
		break;

	case 0x3:	/* Tens Seconds */
		mm58274c->seconds1 = data;
		break;

	case 0x4:	/* Units Minutes */
		mm58274c->minutes2 = data;
		break;

	case 0x5:	/* Tens Minutes */
		mm58274c->minutes1 = data;
		break;

	case 0x6:	/* Units Hours */
		mm58274c->hours2 = data;
		break;

	case 0x7:	/* Tens Hours */
		mm58274c->hours1 = data;
		break;

	case 0x8:	/* Units Days */
		mm58274c->days2 = data;
		break;

	case 0x9:	/* Tens Days */
		mm58274c->days1 = data;
		break;

	case 0xA:	/* Units Months */
		mm58274c->months2 = data;
		break;

	case 0xB:	/* Tens Months */
		mm58274c->months1 = data;
		break;

	case 0xC:	/* Units Years */
		mm58274c->years2 = data;
		break;

	case 0xD:	/* Tens Years */
		mm58274c->years1 = data;
		break;

	case 0xE:	/* Day of Week */
		mm58274c->wday = data;
		break;

	case 0xF:	/* Clock Setting & Interrupt Registers */
		if (mm58274c->control & ctl_intsel)
		{
			/* interrupt register (not emulated) */
			mm58274c->int_ctl = data;
			if (! (mm58274c->control & ctl_intstop))
			{
				/* interrupt run */
				attotime period = interrupt_period_table(mm58274c->int_ctl & int_ctl_dly);

				mm58274c->interrupt_timer->adjust(period, 0, mm58274c->int_ctl & int_ctl_rpt ? period : attotime::zero);
			}
		}
		else
		{
			/* clock setting register */
			mm58274c->clk_set = data;
			#if 0
				if (mm58274c->clk_set & clk_set_24)
					/* 24-hour mode */
					mm58274c->clk_set &= ~clk_set_pm;
			#endif
		}
		break;
	}
}


/*
    Set RTC interrupt flag
*/
static TIMER_CALLBACK(rtc_interrupt_callback)
{
	device_t *device = (device_t *)ptr;
	mm58274c_t *mm58274c = get_safe_token(device);
	mm58274c->status |= st_if;
}


/*
    Increment RTC clock (timed interrupt every 1/10s)
*/

static TIMER_CALLBACK(increment_rtc)
{
	device_t *device = (device_t *)ptr;
	mm58274c_t *mm58274c = get_safe_token(device);
	if (! (mm58274c->control & ctl_clkstop))
	{
		mm58274c->status |= st_dcf;

		if ((++mm58274c->tenths) == 10)
		{
			mm58274c->tenths = 0;

			if ((++mm58274c->seconds2) == 10)
			{
				mm58274c->seconds2 = 0;

				if ((++mm58274c->seconds1) == 6)
				{
					mm58274c->seconds1 = 0;

					if ((++mm58274c->minutes2) == 10)
					{
						mm58274c->minutes2 = 0;

						if ((++mm58274c->minutes1) == 6)
						{
							mm58274c->minutes1 = 0;

							if ((++mm58274c->hours2) == 10)
							{
								mm58274c->hours2 = 0;

								mm58274c->hours1++;
							}

							/* handle wrap-around */
							if ((! (mm58274c->clk_set & clk_set_24))
									&& ((mm58274c->hours1*10 + mm58274c->hours2) == 12))
							{
								mm58274c->clk_set ^= clk_set_pm;
							}
							if ((! (mm58274c->clk_set & clk_set_24))
									&& ((mm58274c->hours1*10 + mm58274c->hours2) == 13))
							{
								mm58274c->hours1 = 0;
								mm58274c->hours2 = 1;
							}

							if ((mm58274c->clk_set & clk_set_24)
								&& ((mm58274c->hours1*10 + mm58274c->hours2) == 24))
							{
								mm58274c->hours1 = mm58274c->hours2 = 0;
							}

							/* increment day if needed */
							if ((mm58274c->clk_set & clk_set_24)
								? ((mm58274c->hours1*10 + mm58274c->hours2) == 0)
								: (((mm58274c->hours1*10 + mm58274c->hours2) == 12)
									&& (! (mm58274c->clk_set & clk_set_pm))))
							{
								int days_in_month;

								if ((++mm58274c->days2) == 10)
								{
									mm58274c->days2 = 0;

									mm58274c->days1++;
								}

								if ((++mm58274c->wday) == 8)
									mm58274c->wday = 1;

								{
									static const int days_in_month_array[] =
									{
										31,28,31, 30,31,30,
										31,31,30, 31,30,31
									};

									if (((mm58274c->months1*10 + mm58274c->months2) != 2) || (mm58274c->clk_set & clk_set_leap))
										days_in_month = days_in_month_array[mm58274c->months1*10 + mm58274c->months2 - 1];
									else
										days_in_month = 29;
								}


								if ((mm58274c->days1*10 + mm58274c->days2) == days_in_month+1)
								{
									mm58274c->days1 = 0;
									mm58274c->days2 = 1;

									if ((++mm58274c->months2) == 10)
									{
										mm58274c->months2 = 0;

										mm58274c->months1++;
									}

									if ((mm58274c->months1*10 + mm58274c->months2) == 13)
									{
										mm58274c->months1 = 0;
										mm58274c->months2 = 1;

										mm58274c->clk_set = (mm58274c->clk_set & ~clk_set_leap)
															| ((mm58274c->clk_set + clk_set_leap_inc) & clk_set_leap);

										if ((++mm58274c->years2) == 10)
										{
											mm58274c->years2 = 0;

											if ((++mm58274c->years1) == 10)
												mm58274c->years1 = 0;
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

/* Device Interface */

static DEVICE_START( mm58274c )
{
	mm58274c_t *mm58274c = get_safe_token(device);

	// validate arguments
	assert(device != NULL);
	assert(device->tag() != NULL);
	assert(device->static_config() != NULL);

	mm58274c->intf = (const mm58274c_interface*)device->static_config();
	// register for state saving
	state_save_register_item(device->machine(), "mm58274c", device->tag(), 0, mm58274c->status);
	state_save_register_item(device->machine(), "mm58274c", device->tag(), 0, mm58274c->control);
	state_save_register_item(device->machine(), "mm58274c", device->tag(), 0, mm58274c->clk_set);
	state_save_register_item(device->machine(), "mm58274c", device->tag(), 0, mm58274c->int_ctl);
	state_save_register_item(device->machine(), "mm58274c", device->tag(), 0, mm58274c->wday);
	state_save_register_item(device->machine(), "mm58274c", device->tag(), 0, mm58274c->years1);
	state_save_register_item(device->machine(), "mm58274c", device->tag(), 0, mm58274c->years2);
	state_save_register_item(device->machine(), "mm58274c", device->tag(), 0, mm58274c->months1);
	state_save_register_item(device->machine(), "mm58274c", device->tag(), 0, mm58274c->months2);
	state_save_register_item(device->machine(), "mm58274c", device->tag(), 0, mm58274c->days1);
	state_save_register_item(device->machine(), "mm58274c", device->tag(), 0, mm58274c->days2);
	state_save_register_item(device->machine(), "mm58274c", device->tag(), 0, mm58274c->hours1);
	state_save_register_item(device->machine(), "mm58274c", device->tag(), 0, mm58274c->hours2);
	state_save_register_item(device->machine(), "mm58274c", device->tag(), 0, mm58274c->minutes1);
	state_save_register_item(device->machine(), "mm58274c", device->tag(), 0, mm58274c->minutes2);
	state_save_register_item(device->machine(), "mm58274c", device->tag(), 0, mm58274c->seconds1);
	state_save_register_item(device->machine(), "mm58274c", device->tag(), 0, mm58274c->seconds2);
	state_save_register_item(device->machine(), "mm58274c", device->tag(), 0, mm58274c->tenths);

	mm58274c->increment_rtc = device->machine().scheduler().timer_alloc(FUNC(increment_rtc), ((void*)device));
	mm58274c->increment_rtc->adjust(attotime::zero, 0, attotime::from_msec(100));
	mm58274c->interrupt_timer = device->machine().scheduler().timer_alloc(FUNC(rtc_interrupt_callback), ((void*)device));
}


static DEVICE_RESET( mm58274c )
{
	mm58274c_t *mm58274c = get_safe_token(device);
	system_time systime;

	/* get the current date/time from the core */
	device->machine().current_datetime(systime);

	mm58274c->clk_set = systime.local_time.year & 3 << 2;
	if (mm58274c->intf->mode24)
		mm58274c->clk_set |= clk_set_24;

	/* The clock count starts on 1st January 1900 */
	mm58274c->wday = 1 + ((systime.local_time.weekday - mm58274c->intf->day1)%7);
	mm58274c->years1 = (systime.local_time.year / 10) % 10;
	mm58274c->years2 = systime.local_time.year % 10;
	mm58274c->months1 = (systime.local_time.month + 1) / 10;
	mm58274c->months2 = (systime.local_time.month + 1) % 10;
	mm58274c->days1 = systime.local_time.mday / 10;
	mm58274c->days2 = systime.local_time.mday % 10;
	if (!mm58274c->intf->mode24)
	{
		/* 12-hour mode */
		if (systime.local_time.hour > 12)
		{
			systime.local_time.hour -= 12;
			mm58274c->clk_set |= clk_set_pm;
		}
		if (systime.local_time.hour == 0)
			systime.local_time.hour = 12;
	}
	mm58274c->hours1 = systime.local_time.hour / 10;
	mm58274c->hours2 = systime.local_time.hour % 10;
	mm58274c->minutes1 = systime.local_time.minute / 10;
	mm58274c->minutes2 = systime.local_time.minute % 10;
	mm58274c->seconds1 = systime.local_time.second / 10;
	mm58274c->seconds2 = systime.local_time.second % 10;
	mm58274c->tenths = 0;
}

DEVICE_GET_INFO( mm58274c )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(mm58274c_t);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(mm58274c);	break;
		case DEVINFO_FCT_STOP:							/* Nothing */								break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(mm58274c);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "National Semiconductor MM58274C");break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "National Semiconductor MM58274C");break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);							break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright MESS Team");			break;
	}
}

const device_type MM58274C = &device_creator<mm58274c_device>;

mm58274c_device::mm58274c_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MM58274C, "National Semiconductor MM58274C", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(mm58274c_t));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mm58274c_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mm58274c_device::device_start()
{
	DEVICE_START_NAME( mm58274c )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mm58274c_device::device_reset()
{
	DEVICE_RESET_NAME( mm58274c )(this);
}


