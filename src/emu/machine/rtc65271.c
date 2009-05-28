/*
    rtc65271 emulation

    This chip is an RTC for computer built by Epson and Spezial-Electronic (I
    think SE is the second source here).

    Reference:
    * Realtime Clock Module RTC-65271 Application Manual
        <http://www.bgmicro.com/pdf/rtc65271.pdf>

    Todo:
    * Support square wave pin output?
    * Support DSE mode?

    Raphael Nabet, 2003-2004
*/

#include "driver.h"
#include "rtc65271.h"

static void field_interrupts(void);
static TIMER_CALLBACK( rtc_SQW_callback );
static TIMER_CALLBACK( rtc_begin_update_callback );
static TIMER_CALLBACK( rtc_end_update_callback );

/* Delay between the beginning (UIP asserted) and the end (UIP cleared and
update interrupt asserted) of the update cycle */
#define UPDATE_CYCLE_TIME ATTOTIME_IN_USEC(1984)
/* Delay between the assertion of UIP and the effective start of the update
cycle */
/*#define UPDATE_CYCLE_DELAY ATTOTIME_IN_USEC(244)*/

static struct
{
	running_machine *machine;

	/* 64 8-bit registers (10 clock registers, 4 control/status registers, and
    50 bytes of user RAM) */
	UINT8 regs[64];
	int cur_reg;

	/* extended RAM: 4kbytes of battery-backed RAM (in pages of 32 bytes) */
	UINT8 *xram;
	int cur_xram_page;

	/* update timer: called every second */
	emu_timer *update_timer;

	/* SQW timer: called every periodic clock half-period */
	emu_timer *SQW_timer;
	int SQW_internal_state;

	/* callback called when interrupt pin state changes (may be NULL) */
	void (*interrupt_callback)(running_machine *machine, int state);
} rtc;

enum
{
	reg_second = 0,
	reg_alarm_second,
	reg_minute,
	reg_alarm_minute,
	reg_hour,
	reg_alarm_hour,
	reg_weekday,
	reg_monthday,
	reg_month,
	reg_year,
	reg_A,
	reg_B,
	reg_C,
	reg_D
};

enum
{
	reg_A_UIP	= 0x80,
	reg_A_DV	= 0x70,
	reg_A_RS	= 0x0F,

	reg_B_SET	= 0x80,
	reg_B_PIE	= 0x40,
	reg_B_AIE	= 0x20,
	reg_B_UIE	= 0x10,
	reg_B_SQW	= 0x08,
	reg_B_DM	= 0x04,
	reg_B_24h	= 0x02,
	reg_B_DSE	= 0x01,

	reg_C_IRQF	= 0x80,
	reg_C_PF	= 0x40,
	reg_C_AF	= 0x20,
	reg_C_UF	= 0x10,

	reg_D_VRT	= 0x80
};

static const int SQW_freq_table[16] =
{
	0,
	256,
	128,
	8192,
	4096,
	2048,
	1024,
	512,
	256,
	128,
	64,
	32,
	16,
	8,
	4,
	2,
};


/*
    BCD utilities
*/

/*
    Increment a binary-encoded UINT8
*/
static UINT8 increment_binary(UINT8 data)
{
	return data+1;
}


/*
    Increment a BCD-encoded UINT8
*/
static UINT8 increment_BCD(UINT8 data)
{
	if ((data & 0x0f) < 0x09)
	{
		if ((data & 0xf0) < 0xa0)
			data++;
		else
			data = data + 0x01 - 0xa0;
	}
	else
	{
		if ((data & 0xf0) < 0xa0)
			data = data - 0x09 + 0x10;
		else
			data = data - 0x09 - 0x90;
	}
	return data;
}


/*
    Convert a binary-encoded UINT8 to BCD
*/
static UINT8 binary_to_BCD(UINT8 data)
{
	data %= 100;

	return ((data / 10) << 4) | (data %10);
}


/*
    Convert a BCD-encoded UINT8 to binary
*/
static UINT8 BCD_to_binary(UINT8 data)
{
	if ((data & 0x0f) >= 0x0a)
		data = data - 0x0a + 0x10;
	if ((data & 0xf0) >= 0xa0)
		data = data - 0xa0;

	return (data & 0x0f) + (((data & 0xf0) >> 4) * 10);
}


/*
    Public functions
*/


/*
    load the SRAM and register contents from file
*/
int rtc65271_file_load(running_machine *machine, mame_file *file)
{
	UINT8 buf;


	/* version flag */
	if (mame_fread(file, & buf, 1) != 1)
		return 1;
	if (buf != 0)
		return 1;

	/* control registers */
	if (mame_fread(file, &buf, 1) != 1)
		return 1;
	rtc.regs[reg_A] = buf & (reg_A_DV /*| reg_A_RS*/);
	if (mame_fread(file, &buf, 1) != 1)
		return 1;
	rtc.regs[reg_B] = buf & (reg_B_SET | reg_B_DM | reg_B_24h | reg_B_DSE);

	/* alarm registers */
	if (mame_fread(file, &rtc.regs[reg_alarm_second], 1) != 1)
		return 1;
	if (mame_fread(file, &rtc.regs[reg_alarm_minute], 1) != 1)
		return 1;
	if (mame_fread(file, &rtc.regs[reg_alarm_hour], 1) != 1)
		return 1;

	/* user RAM */
	if (mame_fread(file, rtc.regs+14, 50) != 50)
		return 1;

	/* extended RAM */
	if (mame_fread(file, rtc.xram, 4096) != 4096)
		return 1;

	rtc.regs[reg_D] |= reg_D_VRT;	/* the data was backed up successfully */
	/*rtc.dirty = FALSE;*/

	{
		mame_system_time systime;

		/* get the current date/time from the core */
		mame_get_current_datetime(machine, &systime);

		/* set clock registers */
		rtc.regs[reg_second] = systime.local_time.second;
		rtc.regs[reg_minute] = systime.local_time.minute;
		if (rtc.regs[reg_B] & reg_B_24h)
			/* 24-hour mode */
			rtc.regs[reg_hour] = systime.local_time.hour;
		else
		{	/* 12-hour mode */
			if (systime.local_time.hour >= 12)
			{
				rtc.regs[reg_hour] = 0x80;
				systime.local_time.hour -= 12;
			}
			else
				rtc.regs[reg_hour] = 0;
			rtc.regs[reg_hour] |= systime.local_time.hour ? systime.local_time.hour : 12;
		}
		rtc.regs[reg_weekday] = systime.local_time.weekday + 1;
		rtc.regs[reg_monthday] = systime.local_time.mday;
		rtc.regs[reg_month] = systime.local_time.month + 1;
		rtc.regs[reg_year] = systime.local_time.year % 100;
		if (! (rtc.regs[reg_B] & reg_B_DM))
		{	/* BCD mode */
			rtc.regs[reg_second] = binary_to_BCD(rtc.regs[reg_second]);
			rtc.regs[reg_minute] = binary_to_BCD(rtc.regs[reg_minute]);
			rtc.regs[reg_hour] = (rtc.regs[reg_hour] & 0x80) | binary_to_BCD(rtc.regs[reg_hour] & 0x7f);
			/*rtc.regs[reg_weekday] = binary_to_BCD(rtc.regs[reg_weekday]);*/
			rtc.regs[reg_monthday] = binary_to_BCD(rtc.regs[reg_monthday]);
			rtc.regs[reg_month] = binary_to_BCD(rtc.regs[reg_month]);
			rtc.regs[reg_year] = binary_to_BCD(rtc.regs[reg_year]);
		}
	}

	return 0;
}

/*
    save the SRAM and register contents to file
*/
int rtc65271_file_save(mame_file *file)
{
	UINT8 buf;


	/* version flag */
	buf = 0;
	if (mame_fwrite(file, & buf, 1) != 1)
		return 1;

	/* control registers */
	buf = rtc.regs[reg_A] & (reg_A_DV | reg_A_RS);
	if (mame_fwrite(file, &buf, 1) != 1)
		return 1;
	buf = rtc.regs[reg_B] & (reg_B_SET | reg_B_DM | reg_B_24h | reg_B_DSE);
	if (mame_fwrite(file, &buf, 1) != 1)
		return 1;

	/* alarm registers */
	if (mame_fwrite(file, &rtc.regs[reg_alarm_second], 1) != 1)
		return 1;
	if (mame_fwrite(file, &rtc.regs[reg_alarm_minute], 1) != 1)
		return 1;
	if (mame_fwrite(file, &rtc.regs[reg_alarm_hour], 1) != 1)
		return 1;

	/* user RAM */
	if (mame_fwrite(file, rtc.regs+14, 50) != 50)
		return 1;

	/* extended RAM */
	if (mame_fwrite(file, rtc.xram, 4096) != 4096)
		return 1;

	return 0;
}

/*
    Initialize clock

    xram: pointer to 4kb RAM area
    interrupt_callback: callback called when interrupt pin state changes (may
        be NULL)
*/
void rtc65271_init(running_machine *machine, UINT8 *xram, void (*interrupt_callback)(running_machine *machine, int state))
{
	memset(&rtc, 0, sizeof(rtc));

	rtc.machine = machine;
	rtc.xram = xram;

	rtc.update_timer = timer_alloc(machine, rtc_begin_update_callback, NULL);
	timer_adjust_periodic(rtc.update_timer, ATTOTIME_IN_SEC(1), 0, ATTOTIME_IN_SEC(1));
	rtc.SQW_timer = timer_alloc(machine, rtc_SQW_callback, NULL);
	rtc.interrupt_callback = interrupt_callback;
}

/*
    Read a byte from clock

    xramsel: select RTC register if 0, XRAM if 1
    offset: address (A0-A5 pins)
*/
UINT8 rtc65271_r(int xramsel, offs_t offset)
{
	int reply;

	if (xramsel)
	{
		if (offset & 0x20)
			/* XRAM page register */
			reply = rtc.cur_xram_page;
		else
			/* XRAM data */
			reply = rtc.xram[(offset & 0x1f) + 0x0020*rtc.cur_xram_page];
	}
	else
	{
		if (offset & 0x01)
			/* data register */
			switch (rtc.cur_reg)
			{
			case reg_C:
				reply = rtc.regs[rtc.cur_reg];
				rtc.regs[rtc.cur_reg] = 0;
				field_interrupts();
				break;
			case reg_D:
				reply = rtc.regs[rtc.cur_reg];
				rtc.regs[rtc.cur_reg] = /*0*/reg_D_VRT;	/* set VRT flag so that the computer does not complain that the battery is low */
				break;

			default:
				reply = rtc.regs[rtc.cur_reg];
				break;
			}
		else
			/* indirect address register */
			reply = rtc.cur_reg;
	}

	return reply;
}

READ8_HANDLER( rtc65271_rtc_r )
{
	return rtc65271_r( 0, offset );
}

READ8_HANDLER( rtc65271_xram_r )
{
	return rtc65271_r( 1, offset );
}

/*
    Write a byte to clock

    xramsel: select RTC register if 0, XRAM if 1
    offset: address (A0-A5 pins)
*/
void rtc65271_w(int xramsel, offs_t offset, UINT8 data)
{
	if (xramsel)
	{
		if (offset & 0x20)
			/* XRAM page register */
			rtc.cur_xram_page = data & 0x7f;
		else
			/* XRAM data */
			rtc.xram[(offset & 0x1f) + 0x0020*rtc.cur_xram_page] = data;
	}
	else
	{
		if (offset & 0x01)
			/* data register */
			switch (rtc.cur_reg)
			{
			case reg_second:
				/* the data sheet says bit 7 is read-only.  (I have no idea of
                the reason why it is.) */
				rtc.regs[reg_second] = data & 0x7f;
				break;

			case reg_A:
				if ((data & reg_A_RS) != (rtc.regs[rtc.cur_reg] & reg_A_RS))
				{
					if (data & reg_A_RS)
					{
						attotime period = ATTOTIME_IN_HZ(SQW_freq_table[data & reg_A_RS]);
						attotime half_period = attotime_div(period, 2);
						attotime elapsed = timer_timeelapsed(rtc.update_timer);

						if (attotime_compare(half_period, elapsed) > 0)
							timer_adjust_oneshot(rtc.SQW_timer, attotime_sub(half_period, elapsed), 0);
						else
							timer_adjust_oneshot(rtc.SQW_timer, half_period, 0);
					}
					else
					{
						rtc.SQW_internal_state = 0;	/* right??? */

						/* Stop the divider used for SQW and periodic interrupts. */
						timer_adjust_oneshot(rtc.SQW_timer, attotime_never, 0);
					}
				}
				/* The UIP bit is read-only */
				rtc.regs[reg_A] = (data & ~reg_A_UIP) | (rtc.regs[reg_A] & reg_A_UIP);
				break;

			case reg_B:
				rtc.regs[rtc.cur_reg] = data;
				if (data & reg_B_SET)
				{
					/* if we are in SET mode, clear update cycle */
					rtc.regs[reg_A] &= ~reg_A_UIP;
					rtc.regs[reg_B] &= ~reg_B_UIE;	/* the data sheet tells this, but I wonder how much sense it makes */
					field_interrupts();
				}
				break;

			case reg_C:
			case reg_D:
				break;

			default:
				rtc.regs[rtc.cur_reg] = data;
				break;
			}
		else
			/* indirect address register */
			rtc.cur_reg = data & 0x3f;
	}
}

WRITE8_HANDLER( rtc65271_rtc_w )
{
	rtc65271_w( 0, offset, data );
}

WRITE8_HANDLER( rtc65271_xram_w )
{
	rtc65271_w( 1, offset, data );
}

static void field_interrupts(void)
{
	if (rtc.regs[reg_C] & rtc.regs[reg_B] & (reg_C_PF | reg_C_AF | reg_C_UF))
	{
		rtc.regs[reg_C] |= reg_C_IRQF;
		if (rtc.interrupt_callback)
			rtc.interrupt_callback(rtc.machine, 1);
	}
	else
	{
		rtc.regs[reg_C] &= ~reg_C_IRQF;
		if (rtc.interrupt_callback)
			rtc.interrupt_callback(rtc.machine, 0);
	}
}


/*
    Timer handlers
*/

/*
    Update SQW output state each half-period and assert periodic interrupt each
    period.
*/
static TIMER_CALLBACK( rtc_SQW_callback )
{
	attotime half_period;

	rtc.SQW_internal_state = ! rtc.SQW_internal_state;
	if (! rtc.SQW_internal_state)
	{
		/* high-to-low??? transition -> interrupt (or should it be low-to-high?) */
		rtc.regs[reg_C] |= reg_C_PF;
		field_interrupts();
	}

	half_period = attotime_div(ATTOTIME_IN_HZ(SQW_freq_table[rtc.regs[reg_A] & reg_A_RS]), 2);
	timer_adjust_oneshot(rtc.SQW_timer, half_period, 0);
}

/*
    Begin update cycle (called every second)
*/
static TIMER_CALLBACK( rtc_begin_update_callback )
{
	if (((rtc.regs[reg_A] & reg_A_DV) == 0x20) && ! (rtc.regs[reg_B] & reg_B_SET))
	{
		rtc.regs[reg_A] |= reg_A_UIP;

		/* schedule end of update cycle */
		timer_set(machine, UPDATE_CYCLE_TIME, NULL, 0, rtc_end_update_callback);
	}
}

/*
    End update cycle (called UPDATE_CYCLE_TIME = 1948us after start of update
    cycle)
*/
static TIMER_CALLBACK( rtc_end_update_callback )
{
	static const int days_in_month_table[12] =
	{
		31,28,31, 30,31,30,
		31,31,30, 31,30,31
	};
	UINT8 (*increment)(UINT8 data);
	int c59, c23, c12, c11, c29;

	if (! (rtc.regs[reg_A] & reg_A_UIP))
		/* abort if update cycle has been canceled */
		return;

	if (rtc.regs[reg_B] & reg_B_DM)
	{
		/* binary mode */
		increment = increment_binary;
		c59 = 59;
		c23 = 23;
		c12 = 12;
		c11 = 11;
		c29 = 29;
	}
	else
	{
		/* BCD mode */
		increment = increment_BCD;
		c59 = 0x59;
		c23 = 0x23;
		c12 = 0x12;
		c11 = 0x11;
		c29 = 0x29;
	}

	/* increment second */
	if (rtc.regs[reg_second] < c59)
		rtc.regs[reg_second] = (*increment)(rtc.regs[reg_second]);
	else
	{
		rtc.regs[reg_second] = 0;

		/* increment minute */
		if (rtc.regs[reg_minute] < c59)
			rtc.regs[reg_minute] = (*increment)(rtc.regs[reg_minute]);
		else
		{
			rtc.regs[reg_minute] = 0;

			/* increment hour */
			if (rtc.regs[reg_B] & reg_B_24h)
			{
				/* 24 hour mode */
				if (rtc.regs[reg_hour] < c23)
					rtc.regs[reg_hour] = (*increment)(rtc.regs[reg_hour]);
				else
					rtc.regs[reg_hour] = 0;
			}
			else
			{
				/* 12 hour mode */
				if (rtc.regs[reg_hour] < c12)
				{
					if ((rtc.regs[reg_hour] & 0x7f) == c11)
						rtc.regs[reg_hour] ^= 0x80;
					rtc.regs[reg_hour] = ((*increment)(rtc.regs[reg_hour] & 0x7f) & 0x7f)
											| (rtc.regs[reg_hour] & 0x80);
				}
				else
					rtc.regs[reg_hour] = 1 | (rtc.regs[reg_hour] & 0x80);
			}

			/* increment day if needed */
			if (rtc.regs[reg_hour] == ((rtc.regs[reg_B] & reg_B_24h) ? 0 : c12))
			{
				/* increment day */
				int days_in_month;

				if (rtc.regs[reg_weekday] < 7)
					rtc.regs[reg_weekday]++;
				else
					rtc.regs[reg_weekday] = 1;

				if ((rtc.regs[reg_month] != 2) || (rtc.regs[reg_year] & 0x03))
				{
					if (rtc.regs[reg_B] & reg_B_DM)
					{
						/* binary mode */
						days_in_month = days_in_month_table[rtc.regs[reg_month] - 1];
					}
					else
					{
						/* BCD mode */
						days_in_month = binary_to_BCD(days_in_month_table[BCD_to_binary(rtc.regs[reg_month]) - 1]);
					}
				}
				else
					days_in_month = c29;

				if (rtc.regs[reg_monthday] < days_in_month)
					rtc.regs[reg_monthday] = (*increment)(rtc.regs[reg_monthday]);
				else
				{
					/* increment month */
					rtc.regs[reg_monthday] = 1;

					if (rtc.regs[reg_month] < c12)
						rtc.regs[reg_month] = (*increment)(rtc.regs[reg_month]);
					else
					{
						/* increment year */
						rtc.regs[reg_month] = 1;

						if (rtc.regs[reg_B] & reg_B_DM)
						{
							/* binary mode */
							if (rtc.regs[reg_year] < 99)
								rtc.regs[reg_year]++;
							else
								rtc.regs[reg_year] = 0;
						}
						else
						{
							/* BCD mode */
							rtc.regs[reg_year] = increment_BCD(rtc.regs[reg_year]);
						}
					}
				}
			}
		}
	}

	rtc.regs[reg_A] &= ~reg_A_UIP;
	rtc.regs[reg_C] |= reg_C_UF;

	/* test for alarm (values in range 0xc0-0xff mean "don't care") */
	if ((((rtc.regs[reg_alarm_second] & 0xc0) == 0xc0) || (rtc.regs[reg_alarm_second] == rtc.regs[reg_second]))
			&& (((rtc.regs[reg_alarm_minute] & 0xc0) == 0xc0) || (rtc.regs[reg_alarm_minute] == rtc.regs[reg_minute]))
			&& (((rtc.regs[reg_alarm_hour] & 0xc0) == 0xc0) || (rtc.regs[reg_alarm_hour] == rtc.regs[reg_hour])))
		rtc.regs[reg_C] |= reg_C_AF;

	field_interrupts();
}
