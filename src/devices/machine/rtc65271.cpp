// license:BSD-3-Clause
// copyright-holders:Raphael Nabet, R. Belmont
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
    R. Belmont, 2012
*/

#include "emu.h"
#include "rtc65271.h"

/* Delay between the beginning (UIP asserted) and the end (UIP cleared and
update interrupt asserted) of the update cycle */
#define UPDATE_CYCLE_TIME attotime::from_usec(1984)
/* Delay between the assertion of UIP and the effective start of the update
cycle */
/*#define UPDATE_CYCLE_DELAY attotime::from_usec(244)*/

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
	reg_A_UIP   = 0x80,
	reg_A_DV    = 0x70,
	reg_A_RS    = 0x0F,

	reg_B_SET   = 0x80,
	reg_B_PIE   = 0x40,
	reg_B_AIE   = 0x20,
	reg_B_UIE   = 0x10,
	reg_B_SQW   = 0x08,
	reg_B_DM    = 0x04,
	reg_B_24h   = 0x02,
	reg_B_DSE   = 0x01,

	reg_C_IRQF  = 0x80,
	reg_C_PF    = 0x40,
	reg_C_AF    = 0x20,
	reg_C_UF    = 0x10,

	reg_D_VRT   = 0x80
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

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void rtc65271_device::nvram_default()
{
	memset(m_regs,0, sizeof(m_regs));
	memset(m_xram,0, sizeof(m_xram));

	m_regs[reg_B] |= reg_B_DM;  // Firebeat assumes the chip factory defaults to non-BCD mode (or maybe Konami programs it that way?)
}

//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void rtc65271_device::nvram_read(emu_file &file)
{
	UINT8 buf;

	/* version flag */
	if (file.read(&buf, 1) != 1)
		return;
	if (buf != 0)
		return;

	/* control registers */
	if (file.read(&buf, 1) != 1)
		return;
	m_regs[reg_A] = buf & (reg_A_DV /*| reg_A_RS*/);
	if (file.read(&buf, 1) != 1)
		return;
	m_regs[reg_B] = buf & (reg_B_SET | reg_B_DM | reg_B_24h | reg_B_DSE);

	/* alarm registers */
	if (file.read(&m_regs[reg_alarm_second], 1) != 1)
		return;
	if (file.read(&m_regs[reg_alarm_minute], 1) != 1)
		return;
	if (file.read(&m_regs[reg_alarm_hour], 1) != 1)
		return;

	/* user RAM */
	if (file.read(m_regs+14, 50) != 50)
		return;

	/* extended RAM */
	if (file.read(m_xram, 4096) != 4096)
		return;

	m_regs[reg_D] |= reg_D_VRT; /* the data was backed up successfully */
	/*m_dirty = FALSE;*/

	{
		system_time systime;

		/* get the current date/time from the core */
		machine().current_datetime(systime);

		/* set clock registers */
		m_regs[reg_second] = systime.local_time.second;
		m_regs[reg_minute] = systime.local_time.minute;
		if (m_regs[reg_B] & reg_B_24h)
			/* 24-hour mode */
			m_regs[reg_hour] = systime.local_time.hour;
		else
		{   /* 12-hour mode */
			if (systime.local_time.hour >= 12)
			{
				m_regs[reg_hour] = 0x80;
				systime.local_time.hour -= 12;
			}
			else
			{
				m_regs[reg_hour] = 0;
			}

			// Firebeat indicates non-BCD 12-hour mode has 0-based hour, so 12 AM is 0x00 and 12 PM is 0x80
			m_regs[reg_hour] |= systime.local_time.hour; // ? systime.local_time.hour : 12;
		}
		m_regs[reg_weekday] = systime.local_time.weekday + 1;
		m_regs[reg_monthday] = systime.local_time.mday;
		m_regs[reg_month] = systime.local_time.month + 1;
		m_regs[reg_year] = systime.local_time.year % 100;
		if (! (m_regs[reg_B] & reg_B_DM))
		{   /* BCD mode */
			m_regs[reg_second] = binary_to_BCD(m_regs[reg_second]);
			m_regs[reg_minute] = binary_to_BCD(m_regs[reg_minute]);
			m_regs[reg_hour] = (m_regs[reg_hour] & 0x80) | binary_to_BCD(m_regs[reg_hour] & 0x7f);
			/*m_regs[reg_weekday] = binary_to_BCD(m_regs[reg_weekday]);*/
			m_regs[reg_monthday] = binary_to_BCD(m_regs[reg_monthday]);
			m_regs[reg_month] = binary_to_BCD(m_regs[reg_month]);
			m_regs[reg_year] = binary_to_BCD(m_regs[reg_year]);
		}
	}
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void rtc65271_device::nvram_write(emu_file &file)
{
	UINT8 buf;


	/* version flag */
	buf = 0;
	if (file.write(& buf, 1) != 1)
		return;

	/* control registers */
	buf = m_regs[reg_A] & (reg_A_DV | reg_A_RS);
	if (file.write(&buf, 1) != 1)
		return;
	buf = m_regs[reg_B] & (reg_B_SET | reg_B_DM | reg_B_24h | reg_B_DSE);
	if (file.write(&buf, 1) != 1)
		return;

	/* alarm registers */
	if (file.write(&m_regs[reg_alarm_second], 1) != 1)
		return;
	if (file.write(&m_regs[reg_alarm_minute], 1) != 1)
		return;
	if (file.write(&m_regs[reg_alarm_hour], 1) != 1)
		return;

	/* user RAM */
	if (file.write(m_regs+14, 50) != 50)
		return;

	/* extended RAM */
	if (file.write(m_xram, 4096) != 4096)
		return;
}

/*
    Read a byte from clock

    xramsel: select RTC register if 0, XRAM if 1
    offset: address (A0-A5 pins)
*/
UINT8 rtc65271_device::read(int xramsel, offs_t offset)
{
	int reply;

	if (xramsel)
	{
		if (offset & 0x20)
			/* XRAM page register */
			reply = m_cur_xram_page;
		else
			/* XRAM data */
			reply = m_xram[(offset & 0x1f) + 0x0020*m_cur_xram_page];
	}
	else
	{
		if (offset & 0x01)
			/* data register */
			switch (m_cur_reg)
			{
			case reg_A:
				reply = m_regs[m_cur_reg] & ~reg_A_DV;
				reply |= 0x20;  // indicate normal RTC operation
				break;

			case reg_C:
				reply = m_regs[m_cur_reg];
				m_regs[m_cur_reg] = 0;
				field_interrupts();
				break;
			case reg_D:
				reply = m_regs[m_cur_reg];
				m_regs[m_cur_reg] = /*0*/reg_D_VRT; /* set VRT flag so that the computer does not complain that the battery is low */
				break;

			default:
				reply = m_regs[m_cur_reg];
				break;
			}
		else
			/* indirect address register */
			reply = m_cur_reg;
	}

	return reply;
}

READ8_MEMBER( rtc65271_device::rtc_r )
{
	return read(0, offset );
}

READ8_MEMBER( rtc65271_device::xram_r )
{
	return read(1, offset );
}

/*
    Write a byte to clock

    xramsel: select RTC register if 0, XRAM if 1
    offset: address (A0-A5 pins)
*/
void rtc65271_device::write(int xramsel, offs_t offset, UINT8 data)
{
	if (xramsel)
	{
		if (offset & 0x20)
			/* XRAM page register */
			m_cur_xram_page = data & 0x7f;
		else
			/* XRAM data */
			m_xram[(offset & 0x1f) + 0x0020*m_cur_xram_page] = data;
	}
	else
	{
		if (offset & 0x01)
			/* data register */
			switch (m_cur_reg)
			{
			case reg_second:
				/* the data sheet says bit 7 is read-only.  (I have no idea of
				the reason why it is.) */
				m_regs[reg_second] = data & 0x7f;
				break;

			case reg_A:
				if ((data & reg_A_RS) != (m_regs[m_cur_reg] & reg_A_RS))
				{
					if (data & reg_A_RS)
					{
						attotime period = attotime::from_hz(SQW_freq_table[data & reg_A_RS]);
						attotime half_period = period / 2;
						attotime elapsed = m_update_timer->elapsed();

						if (half_period > elapsed)
							m_SQW_timer->adjust(half_period - elapsed);
						else
							m_SQW_timer->adjust(half_period);
					}
					else
					{
						m_SQW_internal_state = 0;   /* right??? */

						/* Stop the divider used for SQW and periodic interrupts. */
						m_SQW_timer->adjust(attotime::never);
					}
				}
				/* The UIP bit is read-only */
				m_regs[reg_A] = (data & ~reg_A_UIP) | (m_regs[reg_A] & reg_A_UIP);
				break;

			case reg_B:
				m_regs[m_cur_reg] = data;
				if (data & reg_B_SET)
				{
					/* if we are in SET mode, clear update cycle */
					m_regs[reg_A] &= ~reg_A_UIP;
					m_regs[reg_B] &= ~reg_B_UIE;    /* the data sheet tells this, but I wonder how much sense it makes */
					field_interrupts();
				}
				break;

			case reg_C:
			case reg_D:
				break;

			default:
				m_regs[m_cur_reg] = data;
				break;
			}
		else
			/* indirect address register */
			m_cur_reg = data & 0x3f;
	}
}

WRITE8_MEMBER( rtc65271_device::rtc_w )
{
	write(0, offset, data );
}

WRITE8_MEMBER( rtc65271_device::xram_w )
{
	write(1, offset, data );
}

void rtc65271_device::field_interrupts()
{
	if (m_regs[reg_C] & m_regs[reg_B] & (reg_C_PF | reg_C_AF | reg_C_UF))
	{
		m_regs[reg_C] |= reg_C_IRQF;
		if (!m_interrupt_cb.isnull())
			m_interrupt_cb(1);
	}
	else
	{
		m_regs[reg_C] &= ~reg_C_IRQF;
		if (!m_interrupt_cb.isnull())
			m_interrupt_cb(0);
	}
}


/*
    Timer handlers
*/
TIMER_CALLBACK( rtc65271_device::rtc_SQW_callback )
{
	rtc65271_device *rtc = reinterpret_cast<rtc65271_device *>(ptr);
	rtc->rtc_SQW_cb();
}

TIMER_CALLBACK( rtc65271_device::rtc_begin_update_callback )
{
	rtc65271_device *rtc = reinterpret_cast<rtc65271_device *>(ptr);
	rtc->rtc_begin_update_cb();
}

TIMER_CALLBACK( rtc65271_device::rtc_end_update_callback )
{
	rtc65271_device *rtc = reinterpret_cast<rtc65271_device *>(ptr);
	rtc->rtc_end_update_cb();
}
/*
    Update SQW output state each half-period and assert periodic interrupt each
    period.
*/
void rtc65271_device::rtc_SQW_cb()
{
	attotime half_period;

	m_SQW_internal_state = ! m_SQW_internal_state;
	if (! m_SQW_internal_state)
	{
		/* high-to-low??? transition -> interrupt (or should it be low-to-high?) */
		m_regs[reg_C] |= reg_C_PF;
		field_interrupts();
	}

	half_period = attotime::from_hz(SQW_freq_table[m_regs[reg_A] & reg_A_RS]) / 2;
	m_SQW_timer->adjust(half_period);
}

/*
    Begin update cycle (called every second)
*/
void rtc65271_device::rtc_begin_update_cb()
{
	if (((m_regs[reg_A] & reg_A_DV) == 0x20) && ! (m_regs[reg_B] & reg_B_SET))
	{
		m_regs[reg_A] |= reg_A_UIP;

		/* schedule end of update cycle */
		machine().scheduler().timer_set(UPDATE_CYCLE_TIME, FUNC(rtc_end_update_callback), 0, (void *)this);
	}
}

/*
    End update cycle (called UPDATE_CYCLE_TIME = 1948us after start of update
    cycle)
*/
void rtc65271_device::rtc_end_update_cb()
{
	static const int days_in_month_table[12] =
	{
		31,28,31, 30,31,30,
		31,31,30, 31,30,31
	};
	UINT8 (*increment)(UINT8 data);
	int c59, c23, c12, c11, c29;

	if (! (m_regs[reg_A] & reg_A_UIP))
		/* abort if update cycle has been canceled */
		return;

	if (m_regs[reg_B] & reg_B_DM)
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
	if (m_regs[reg_second] < c59)
		m_regs[reg_second] = (*increment)(m_regs[reg_second]);
	else
	{
		m_regs[reg_second] = 0;

		/* increment minute */
		if (m_regs[reg_minute] < c59)
			m_regs[reg_minute] = (*increment)(m_regs[reg_minute]);
		else
		{
			m_regs[reg_minute] = 0;

			/* increment hour */
			if (m_regs[reg_B] & reg_B_24h)
			{
				/* 24 hour mode */
				if (m_regs[reg_hour] < c23)
					m_regs[reg_hour] = (*increment)(m_regs[reg_hour]);
				else
					m_regs[reg_hour] = 0;
			}
			else
			{
				/* 12 hour mode */
				if (m_regs[reg_hour] < c12)
				{
					if ((m_regs[reg_hour] & 0x7f) == c11)
						m_regs[reg_hour] ^= 0x80;
					m_regs[reg_hour] = ((*increment)(m_regs[reg_hour] & 0x7f) & 0x7f)
											| (m_regs[reg_hour] & 0x80);
				}
				else
					m_regs[reg_hour] = 1 | (m_regs[reg_hour] & 0x80);
			}

			/* increment day if needed */
			if (m_regs[reg_hour] == ((m_regs[reg_B] & reg_B_24h) ? 0 : c12))
			{
				/* increment day */
				int days_in_month;

				if (m_regs[reg_weekday] < 7)
					m_regs[reg_weekday]++;
				else
					m_regs[reg_weekday] = 1;

				if ((m_regs[reg_month] != 2) || (m_regs[reg_year] & 0x03))
				{
					if (m_regs[reg_B] & reg_B_DM)
					{
						/* binary mode */
						days_in_month = days_in_month_table[m_regs[reg_month] - 1];
					}
					else
					{
						/* BCD mode */
						days_in_month = binary_to_BCD(days_in_month_table[BCD_to_binary(m_regs[reg_month]) - 1]);
					}
				}
				else
					days_in_month = c29;

				if (m_regs[reg_monthday] < days_in_month)
					m_regs[reg_monthday] = (*increment)(m_regs[reg_monthday]);
				else
				{
					/* increment month */
					m_regs[reg_monthday] = 1;

					if (m_regs[reg_month] < c12)
						m_regs[reg_month] = (*increment)(m_regs[reg_month]);
					else
					{
						/* increment year */
						m_regs[reg_month] = 1;

						if (m_regs[reg_B] & reg_B_DM)
						{
							/* binary mode */
							if (m_regs[reg_year] < 99)
								m_regs[reg_year]++;
							else
								m_regs[reg_year] = 0;
						}
						else
						{
							/* BCD mode */
							m_regs[reg_year] = increment_BCD(m_regs[reg_year]);
						}
					}
				}
			}
		}
	}

	m_regs[reg_A] &= ~reg_A_UIP;
	m_regs[reg_C] |= reg_C_UF;

	/* test for alarm (values in range 0xc0-0xff mean "don't care") */
	if ((((m_regs[reg_alarm_second] & 0xc0) == 0xc0) || (m_regs[reg_alarm_second] == m_regs[reg_second]))
			&& (((m_regs[reg_alarm_minute] & 0xc0) == 0xc0) || (m_regs[reg_alarm_minute] == m_regs[reg_minute]))
			&& (((m_regs[reg_alarm_hour] & 0xc0) == 0xc0) || (m_regs[reg_alarm_hour] == m_regs[reg_hour])))
		m_regs[reg_C] |= reg_C_AF;

	field_interrupts();
}

// device type definition
const device_type RTC65271 = &device_creator<rtc65271_device>;

//-------------------------------------------------
//  rtc65271_device - constructor
//-------------------------------------------------

rtc65271_device::rtc65271_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, RTC65271, "RTC-65271", tag, owner, clock, "rtc65271", __FILE__),
		device_nvram_interface(mconfig, *this),
		m_interrupt_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void rtc65271_device::device_start()
{
	m_update_timer = machine().scheduler().timer_alloc(FUNC(rtc_begin_update_callback), (void *)this);
	m_update_timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));
	m_SQW_timer = machine().scheduler().timer_alloc(FUNC(rtc_SQW_callback), (void *)this);
	m_interrupt_cb.resolve();

	save_item(NAME(m_regs));
	save_item(NAME(m_cur_reg));
	save_item(NAME(m_xram));
	save_item(NAME(m_cur_xram_page));
	save_item(NAME(m_SQW_internal_state));
}
