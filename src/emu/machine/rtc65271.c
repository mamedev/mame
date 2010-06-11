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

#include "emu.h"
#include "rtc65271.h"

static void field_interrupts(running_device *device);
static TIMER_CALLBACK( rtc_SQW_callback );
static TIMER_CALLBACK( rtc_begin_update_callback );
static TIMER_CALLBACK( rtc_end_update_callback );

/* Delay between the beginning (UIP asserted) and the end (UIP cleared and
update interrupt asserted) of the update cycle */
#define UPDATE_CYCLE_TIME ATTOTIME_IN_USEC(1984)
/* Delay between the assertion of UIP and the effective start of the update
cycle */
/*#define UPDATE_CYCLE_DELAY ATTOTIME_IN_USEC(244)*/

typedef struct _rtc65271_state rtc65271_state;
struct _rtc65271_state
{
	/* 64 8-bit registers (10 clock registers, 4 control/status registers, and
    50 bytes of user RAM) */
	UINT8 regs[64];
	UINT8 cur_reg;

	/* extended RAM: 4kbytes of battery-backed RAM (in pages of 32 bytes) */
	UINT8 xram[4096];
	UINT8 cur_xram_page;

	/* update timer: called every second */
	emu_timer *update_timer;

	/* SQW timer: called every periodic clock half-period */
	emu_timer *SQW_timer;
	UINT8 SQW_internal_state;

	/* callback called when interrupt pin state changes (may be NULL) */
	void (*interrupt_callback)(running_device *device, int state);
};

INLINE rtc65271_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == RTC65271);

	return (rtc65271_state *)downcast<legacy_device_base *>(device)->token();
}


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
static int rtc65271_file_load(running_device *device, mame_file *file)
{
	rtc65271_state *state = get_safe_token(device);
	UINT8 buf;


	/* version flag */
	if (mame_fread(file, & buf, 1) != 1)
		return 1;
	if (buf != 0)
		return 1;

	/* control registers */
	if (mame_fread(file, &buf, 1) != 1)
		return 1;
	state->regs[reg_A] = buf & (reg_A_DV /*| reg_A_RS*/);
	if (mame_fread(file, &buf, 1) != 1)
		return 1;
	state->regs[reg_B] = buf & (reg_B_SET | reg_B_DM | reg_B_24h | reg_B_DSE);

	/* alarm registers */
	if (mame_fread(file, &state->regs[reg_alarm_second], 1) != 1)
		return 1;
	if (mame_fread(file, &state->regs[reg_alarm_minute], 1) != 1)
		return 1;
	if (mame_fread(file, &state->regs[reg_alarm_hour], 1) != 1)
		return 1;

	/* user RAM */
	if (mame_fread(file, state->regs+14, 50) != 50)
		return 1;

	/* extended RAM */
	if (mame_fread(file, state->xram, 4096) != 4096)
		return 1;

	state->regs[reg_D] |= reg_D_VRT;	/* the data was backed up successfully */
	/*state->dirty = FALSE;*/

	{
		mame_system_time systime;

		/* get the current date/time from the core */
		mame_get_current_datetime(device->machine, &systime);

		/* set clock registers */
		state->regs[reg_second] = systime.local_time.second;
		state->regs[reg_minute] = systime.local_time.minute;
		if (state->regs[reg_B] & reg_B_24h)
			/* 24-hour mode */
			state->regs[reg_hour] = systime.local_time.hour;
		else
		{	/* 12-hour mode */
			if (systime.local_time.hour >= 12)
			{
				state->regs[reg_hour] = 0x80;
				systime.local_time.hour -= 12;
			}
			else
				state->regs[reg_hour] = 0;
			state->regs[reg_hour] |= systime.local_time.hour ? systime.local_time.hour : 12;
		}
		state->regs[reg_weekday] = systime.local_time.weekday + 1;
		state->regs[reg_monthday] = systime.local_time.mday;
		state->regs[reg_month] = systime.local_time.month + 1;
		state->regs[reg_year] = systime.local_time.year % 100;
		if (! (state->regs[reg_B] & reg_B_DM))
		{	/* BCD mode */
			state->regs[reg_second] = binary_to_BCD(state->regs[reg_second]);
			state->regs[reg_minute] = binary_to_BCD(state->regs[reg_minute]);
			state->regs[reg_hour] = (state->regs[reg_hour] & 0x80) | binary_to_BCD(state->regs[reg_hour] & 0x7f);
			/*state->regs[reg_weekday] = binary_to_BCD(state->regs[reg_weekday]);*/
			state->regs[reg_monthday] = binary_to_BCD(state->regs[reg_monthday]);
			state->regs[reg_month] = binary_to_BCD(state->regs[reg_month]);
			state->regs[reg_year] = binary_to_BCD(state->regs[reg_year]);
		}
	}

	return 0;
}

/*
    save the SRAM and register contents to file
*/
static int rtc65271_file_save(running_device *device, mame_file *file)
{
	rtc65271_state *state = get_safe_token(device);
	UINT8 buf;


	/* version flag */
	buf = 0;
	if (mame_fwrite(file, & buf, 1) != 1)
		return 1;

	/* control registers */
	buf = state->regs[reg_A] & (reg_A_DV | reg_A_RS);
	if (mame_fwrite(file, &buf, 1) != 1)
		return 1;
	buf = state->regs[reg_B] & (reg_B_SET | reg_B_DM | reg_B_24h | reg_B_DSE);
	if (mame_fwrite(file, &buf, 1) != 1)
		return 1;

	/* alarm registers */
	if (mame_fwrite(file, &state->regs[reg_alarm_second], 1) != 1)
		return 1;
	if (mame_fwrite(file, &state->regs[reg_alarm_minute], 1) != 1)
		return 1;
	if (mame_fwrite(file, &state->regs[reg_alarm_hour], 1) != 1)
		return 1;

	/* user RAM */
	if (mame_fwrite(file, state->regs+14, 50) != 50)
		return 1;

	/* extended RAM */
	if (mame_fwrite(file, state->xram, 4096) != 4096)
		return 1;

	return 0;
}

/*
    Read a byte from clock

    xramsel: select RTC register if 0, XRAM if 1
    offset: address (A0-A5 pins)
*/
UINT8 rtc65271_r(running_device *device, int xramsel, offs_t offset)
{
	rtc65271_state *state = get_safe_token(device);
	int reply;

	if (xramsel)
	{
		if (offset & 0x20)
			/* XRAM page register */
			reply = state->cur_xram_page;
		else
			/* XRAM data */
			reply = state->xram[(offset & 0x1f) + 0x0020*state->cur_xram_page];
	}
	else
	{
		if (offset & 0x01)
			/* data register */
			switch (state->cur_reg)
			{
			case reg_C:
				reply = state->regs[state->cur_reg];
				state->regs[state->cur_reg] = 0;
				field_interrupts(device);
				break;
			case reg_D:
				reply = state->regs[state->cur_reg];
				state->regs[state->cur_reg] = /*0*/reg_D_VRT;	/* set VRT flag so that the computer does not complain that the battery is low */
				break;

			default:
				reply = state->regs[state->cur_reg];
				break;
			}
		else
			/* indirect address register */
			reply = state->cur_reg;
	}

	return reply;
}

READ8_DEVICE_HANDLER( rtc65271_rtc_r )
{
	return rtc65271_r( device, 0, offset );
}

READ8_DEVICE_HANDLER( rtc65271_xram_r )
{
	return rtc65271_r( device, 1, offset );
}

/*
    Write a byte to clock

    xramsel: select RTC register if 0, XRAM if 1
    offset: address (A0-A5 pins)
*/
void rtc65271_w(running_device *device, int xramsel, offs_t offset, UINT8 data)
{
	rtc65271_state *state = get_safe_token(device);
	if (xramsel)
	{
		if (offset & 0x20)
			/* XRAM page register */
			state->cur_xram_page = data & 0x7f;
		else
			/* XRAM data */
			state->xram[(offset & 0x1f) + 0x0020*state->cur_xram_page] = data;
	}
	else
	{
		if (offset & 0x01)
			/* data register */
			switch (state->cur_reg)
			{
			case reg_second:
				/* the data sheet says bit 7 is read-only.  (I have no idea of
                the reason why it is.) */
				state->regs[reg_second] = data & 0x7f;
				break;

			case reg_A:
				if ((data & reg_A_RS) != (state->regs[state->cur_reg] & reg_A_RS))
				{
					if (data & reg_A_RS)
					{
						attotime period = ATTOTIME_IN_HZ(SQW_freq_table[data & reg_A_RS]);
						attotime half_period = attotime_div(period, 2);
						attotime elapsed = timer_timeelapsed(state->update_timer);

						if (attotime_compare(half_period, elapsed) > 0)
							timer_adjust_oneshot(state->SQW_timer, attotime_sub(half_period, elapsed), 0);
						else
							timer_adjust_oneshot(state->SQW_timer, half_period, 0);
					}
					else
					{
						state->SQW_internal_state = 0;	/* right??? */

						/* Stop the divider used for SQW and periodic interrupts. */
						timer_adjust_oneshot(state->SQW_timer, attotime_never, 0);
					}
				}
				/* The UIP bit is read-only */
				state->regs[reg_A] = (data & ~reg_A_UIP) | (state->regs[reg_A] & reg_A_UIP);
				break;

			case reg_B:
				state->regs[state->cur_reg] = data;
				if (data & reg_B_SET)
				{
					/* if we are in SET mode, clear update cycle */
					state->regs[reg_A] &= ~reg_A_UIP;
					state->regs[reg_B] &= ~reg_B_UIE;	/* the data sheet tells this, but I wonder how much sense it makes */
					field_interrupts(device);
				}
				break;

			case reg_C:
			case reg_D:
				break;

			default:
				state->regs[state->cur_reg] = data;
				break;
			}
		else
			/* indirect address register */
			state->cur_reg = data & 0x3f;
	}
}

WRITE8_DEVICE_HANDLER( rtc65271_rtc_w )
{
	rtc65271_w( device, 0, offset, data );
}

WRITE8_DEVICE_HANDLER( rtc65271_xram_w )
{
	rtc65271_w( device, 1, offset, data );
}

static void field_interrupts(running_device *device)
{
	rtc65271_state *state = get_safe_token(device);

	if (state->regs[reg_C] & state->regs[reg_B] & (reg_C_PF | reg_C_AF | reg_C_UF))
	{
		state->regs[reg_C] |= reg_C_IRQF;
		if (state->interrupt_callback)
			state->interrupt_callback(device, 1);
	}
	else
	{
		state->regs[reg_C] &= ~reg_C_IRQF;
		if (state->interrupt_callback)
			state->interrupt_callback(device, 0);
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
	running_device *device = (running_device *)ptr;
	rtc65271_state *state = get_safe_token(device);
	attotime half_period;

	state->SQW_internal_state = ! state->SQW_internal_state;
	if (! state->SQW_internal_state)
	{
		/* high-to-low??? transition -> interrupt (or should it be low-to-high?) */
		state->regs[reg_C] |= reg_C_PF;
		field_interrupts(device);
	}

	half_period = attotime_div(ATTOTIME_IN_HZ(SQW_freq_table[state->regs[reg_A] & reg_A_RS]), 2);
	timer_adjust_oneshot(state->SQW_timer, half_period, 0);
}

/*
    Begin update cycle (called every second)
*/
static TIMER_CALLBACK( rtc_begin_update_callback )
{
	running_device *device = (running_device *)ptr;
	rtc65271_state *state = get_safe_token(device);

	if (((state->regs[reg_A] & reg_A_DV) == 0x20) && ! (state->regs[reg_B] & reg_B_SET))
	{
		state->regs[reg_A] |= reg_A_UIP;

		/* schedule end of update cycle */
		timer_set(device->machine, UPDATE_CYCLE_TIME, (void *)device, 0, rtc_end_update_callback);
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
	running_device *device = (running_device *)ptr;
	rtc65271_state *state = get_safe_token(device);
	UINT8 (*increment)(UINT8 data);
	int c59, c23, c12, c11, c29;

	if (! (state->regs[reg_A] & reg_A_UIP))
		/* abort if update cycle has been canceled */
		return;

	if (state->regs[reg_B] & reg_B_DM)
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
	if (state->regs[reg_second] < c59)
		state->regs[reg_second] = (*increment)(state->regs[reg_second]);
	else
	{
		state->regs[reg_second] = 0;

		/* increment minute */
		if (state->regs[reg_minute] < c59)
			state->regs[reg_minute] = (*increment)(state->regs[reg_minute]);
		else
		{
			state->regs[reg_minute] = 0;

			/* increment hour */
			if (state->regs[reg_B] & reg_B_24h)
			{
				/* 24 hour mode */
				if (state->regs[reg_hour] < c23)
					state->regs[reg_hour] = (*increment)(state->regs[reg_hour]);
				else
					state->regs[reg_hour] = 0;
			}
			else
			{
				/* 12 hour mode */
				if (state->regs[reg_hour] < c12)
				{
					if ((state->regs[reg_hour] & 0x7f) == c11)
						state->regs[reg_hour] ^= 0x80;
					state->regs[reg_hour] = ((*increment)(state->regs[reg_hour] & 0x7f) & 0x7f)
											| (state->regs[reg_hour] & 0x80);
				}
				else
					state->regs[reg_hour] = 1 | (state->regs[reg_hour] & 0x80);
			}

			/* increment day if needed */
			if (state->regs[reg_hour] == ((state->regs[reg_B] & reg_B_24h) ? 0 : c12))
			{
				/* increment day */
				int days_in_month;

				if (state->regs[reg_weekday] < 7)
					state->regs[reg_weekday]++;
				else
					state->regs[reg_weekday] = 1;

				if ((state->regs[reg_month] != 2) || (state->regs[reg_year] & 0x03))
				{
					if (state->regs[reg_B] & reg_B_DM)
					{
						/* binary mode */
						days_in_month = days_in_month_table[state->regs[reg_month] - 1];
					}
					else
					{
						/* BCD mode */
						days_in_month = binary_to_BCD(days_in_month_table[BCD_to_binary(state->regs[reg_month]) - 1]);
					}
				}
				else
					days_in_month = c29;

				if (state->regs[reg_monthday] < days_in_month)
					state->regs[reg_monthday] = (*increment)(state->regs[reg_monthday]);
				else
				{
					/* increment month */
					state->regs[reg_monthday] = 1;

					if (state->regs[reg_month] < c12)
						state->regs[reg_month] = (*increment)(state->regs[reg_month]);
					else
					{
						/* increment year */
						state->regs[reg_month] = 1;

						if (state->regs[reg_B] & reg_B_DM)
						{
							/* binary mode */
							if (state->regs[reg_year] < 99)
								state->regs[reg_year]++;
							else
								state->regs[reg_year] = 0;
						}
						else
						{
							/* BCD mode */
							state->regs[reg_year] = increment_BCD(state->regs[reg_year]);
						}
					}
				}
			}
		}
	}

	state->regs[reg_A] &= ~reg_A_UIP;
	state->regs[reg_C] |= reg_C_UF;

	/* test for alarm (values in range 0xc0-0xff mean "don't care") */
	if ((((state->regs[reg_alarm_second] & 0xc0) == 0xc0) || (state->regs[reg_alarm_second] == state->regs[reg_second]))
			&& (((state->regs[reg_alarm_minute] & 0xc0) == 0xc0) || (state->regs[reg_alarm_minute] == state->regs[reg_minute]))
			&& (((state->regs[reg_alarm_hour] & 0xc0) == 0xc0) || (state->regs[reg_alarm_hour] == state->regs[reg_hour])))
		state->regs[reg_C] |= reg_C_AF;

	field_interrupts(device);
}

/*
    Initialize clock

    interrupt_callback: callback called when interrupt pin state changes (may
        be NULL)
*/

static DEVICE_START( rtc65271 )
{
	rtc65271_config *config = (rtc65271_config *)downcast<const legacy_device_config_base &>(device->baseconfig()).inline_config();
	rtc65271_state *state = get_safe_token(device);

	state->update_timer = timer_alloc(device->machine, rtc_begin_update_callback, (void *)device);
	timer_adjust_periodic(state->update_timer, ATTOTIME_IN_SEC(1), 0, ATTOTIME_IN_SEC(1));
	state->SQW_timer = timer_alloc(device->machine, rtc_SQW_callback, (void *)device);
	state->interrupt_callback = config->interrupt_callback;

	state_save_register_device_item_array(device, 0, state->regs);
	state_save_register_device_item(device, 0, state->cur_reg);
	state_save_register_device_item_array(device, 0, state->xram);
	state_save_register_device_item(device, 0, state->cur_xram_page);
	state_save_register_device_item(device, 0, state->SQW_internal_state);
}


static DEVICE_NVRAM( rtc65271 )
{
	if (read_or_write)
		rtc65271_file_save(device, file);
	else if (file)
		rtc65271_file_load(device, file);
}


static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)		p##rtc65271##s
#define DEVTEMPLATE_FEATURES	DT_HAS_START | DT_HAS_NVRAM | DT_HAS_INLINE_CONFIG
#define DEVTEMPLATE_NAME		"RTC65271"
#define DEVTEMPLATE_FAMILY		"RTC"
#include "devtempl.h"


DEFINE_LEGACY_NVRAM_DEVICE(RTC65271, rtc65271);
