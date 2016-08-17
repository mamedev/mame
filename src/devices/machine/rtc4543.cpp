// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    rtc4543.c - Epson R4543 real-time clock chip emulation
    by R. Belmont

    JRC 6355E / NJU6355E is basically similar, but order of registers
    is reversed and readouts happen on falling CLK edge.

    The clock's seven registers are read out as 52 consecutive bits of
    data, with the middle register being only 4 bits wide. The bits
    are numbered 0-27 and 32-55 here for implementation convenience.

**********************************************************************/

#include "rtc4543.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define VERBOSE 0

const char *rtc4543_device::s_reg_names[7] =
{
	"second",
	"minute",
	"hour",
	"day of the week",
	"day",
	"month",
	"year"
};


//**************************************************************************
//  RTC4543 DEVICE
//**************************************************************************

// device type definition
const device_type RTC4543 = &device_creator<rtc4543_device>;


//-------------------------------------------------
//  rtc4543_device - constructor
//-------------------------------------------------

rtc4543_device::rtc4543_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, RTC4543, "R4543 RTC", tag, owner, clock, "rtc4543", __FILE__),
		device_rtc_interface(mconfig, *this),
		data_cb(*this), m_ce(0), m_clk(0), m_wr(0), m_data(0), m_curbit(0), m_clock_timer(nullptr)
{
}

rtc4543_device::rtc4543_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *filename)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, filename),
		device_rtc_interface(mconfig, *this),
		data_cb(*this), m_ce(0), m_clk(0), m_wr(0), m_data(0), m_curbit(0), m_clock_timer(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void rtc4543_device::device_start()
{
	data_cb.resolve_safe();

	// allocate timers
	m_clock_timer = timer_alloc();
	m_clock_timer->adjust(attotime::from_hz(clock() / 32768), 0, attotime::from_hz(clock() / 32768));

	// state saving
	save_item(NAME(m_ce));
	save_item(NAME(m_clk));
	save_item(NAME(m_wr));
	save_item(NAME(m_data));
	save_item(NAME(m_regs));
	save_item(NAME(m_curbit));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void rtc4543_device::device_reset()
{
	set_current_time(machine());

	m_ce = 0;
	m_wr = 0;
	m_clk = 0;
	m_data = 0;
	m_curbit = 0;
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void rtc4543_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	advance_seconds();
}


//-------------------------------------------------
//  rtc_clock_updated -
//-------------------------------------------------

void rtc4543_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	static const int weekday[7] = { 7, 1, 2, 3, 4, 5, 6 };

	m_regs[0] = convert_to_bcd(second);                     // seconds (BCD, 0-59) in bits 0-6, bit 7 = battery low
	m_regs[1] = convert_to_bcd(minute);                     // minutes (BCD, 0-59)
	m_regs[2] = convert_to_bcd(hour);                       // hour (BCD, 0-23)
	m_regs[3] = convert_to_bcd(weekday[day_of_week - 1]);   // day of the week (1-7)
	m_regs[4] = convert_to_bcd(day);                        // day (BCD, 1-31)
	m_regs[5] = convert_to_bcd(month);                      // month (BCD, 1-12)
	m_regs[6] = convert_to_bcd(year % 100);                 // year (BCD, 0-99)
}


//-------------------------------------------------
//  ce_w - chip enable write
//-------------------------------------------------

WRITE_LINE_MEMBER( rtc4543_device::ce_w )
{
	if (!state && m_ce) // complete transfer
	{
		if (VERBOSE) logerror("CE falling edge\n", state);
		ce_falling();
	}
	else if (state && !m_ce) // start new data transfer
	{
		if (VERBOSE) logerror("CE rising edge\n", state);
		ce_rising();
	}

	m_ce = state;

	// timer disabled during writes
	m_clock_timer->enable(!m_ce || !m_wr);
}


//-------------------------------------------------
//  ce_rising - CE rising edge trigger
//-------------------------------------------------

void rtc4543_device::ce_rising()
{
	m_curbit = 0; // force immediate reload of output data
}


//-------------------------------------------------
//  ce_falling - CE falling edge trigger
//-------------------------------------------------

void rtc4543_device::ce_falling()
{
}


//-------------------------------------------------
//  wr_w - data direction line write
//-------------------------------------------------

WRITE_LINE_MEMBER( rtc4543_device::wr_w )
{
	if (VERBOSE && (state != m_wr))
		logerror("WR: %u\n", state);

	m_wr = state;
}


//-------------------------------------------------
//  clk_w - serial clock write
//-------------------------------------------------

WRITE_LINE_MEMBER( rtc4543_device::clk_w )
{
	if (m_ce)
	{
		int bit = m_curbit;
		if (!m_clk && state)
		{
			clk_rising();
			if (VERBOSE) logerror("CLK rising edge (I/O: %u, bit %d)\n", m_data, bit);
		}
		else if (m_clk && !state)
		{
			clk_falling();
			if (VERBOSE) logerror("CLK falling edge (I/O: %u, bit %d)\n", m_data, bit);
		}
	}

	m_clk = state;
}


//-------------------------------------------------
//  clk_rising - CLK rising edge trigger
//-------------------------------------------------

void rtc4543_device::clk_rising()
{
	// note: output data does not change when clk at final bit
	if (m_curbit == 56)
		return;

	// rising edge - read/write data becomes valid here
	if (!m_wr)
		load_bit(m_curbit / 8);
	else
		store_bit(m_curbit / 8);

	advance_bit();

	// update only occurs when a write goes all the way through
	if (m_wr && m_curbit == 56)
		update_effective();
}


//-------------------------------------------------
//  clk_falling - CLK falling edge trigger
//-------------------------------------------------

void rtc4543_device::clk_falling()
{
}


//-------------------------------------------------
//  data_w - I/O write
//-------------------------------------------------

WRITE_LINE_MEMBER( rtc4543_device::data_w )
{
	m_data = state & 1;
}


//-------------------------------------------------
//  data_r - I/O read
//-------------------------------------------------

READ_LINE_MEMBER( rtc4543_device::data_r )
{
	return m_data;
}


//-------------------------------------------------
//  load_bit - serial read from register
//-------------------------------------------------

void rtc4543_device::load_bit(int reg)
{
	assert(reg < ARRAY_LENGTH(m_regs));
	int bit = m_curbit & 7;

	// reload data?
	if (VERBOSE)
	{
		if (bit == 0)
			logerror("RTC sending low digit of %s: %x\n", s_reg_names[reg], m_regs[reg] & 0xf);
		else if (bit == 4)
			logerror("RTC sending high digit of %s: %x\n", s_reg_names[reg], (m_regs[reg] >> 4) & 0xf);
	}

	// shift data bit
	m_data = (m_regs[reg] >> bit) & 1;
	data_cb(m_data);
}


//-------------------------------------------------
//  store_bit - serial write
//-------------------------------------------------

void rtc4543_device::store_bit(int reg)
{
	assert(reg < ARRAY_LENGTH(m_regs));
	int bit = m_curbit & 7;

	m_regs[reg] &= ~(1 << bit);
	m_regs[reg] |= m_data << bit;

	if (VERBOSE)
	{
		if (bit == 7)
			logerror("RTC received high digit of %s: %X\n", s_reg_names[reg], (m_regs[reg] >> 4) & 0xf);
		else if (bit == 3)
			logerror("RTC received low digit of %s: %X\n", s_reg_names[reg], m_regs[reg] & 0xf);
	}
}


//-------------------------------------------------
//  advance_bit - increment the bit counter
//-------------------------------------------------

void rtc4543_device::advance_bit()
{
	m_curbit++;

	// day-of-week register only takes 4 bits
	if (m_curbit == 28)
	{
		// skip 4 bits, Brother Maynard
		m_curbit += 4;
	}
}


//-------------------------------------------------
//  update_effective - update the RTC
//-------------------------------------------------

void rtc4543_device::update_effective()
{
	if (VERBOSE)
		logerror("RTC updated: %02x.%02x.%02x (%01x) %02x:%02x:%02x\n", m_regs[6], m_regs[5], m_regs[4], m_regs[3], m_regs[2], m_regs[1], m_regs[0]);
	set_time(false,
		bcd_to_integer(m_regs[6]),      // year
		bcd_to_integer(m_regs[5]),      // month
		bcd_to_integer(m_regs[4]),      // day
		(m_regs[3] % 7) + 1,            // day of week
		bcd_to_integer(m_regs[2]),      // hour
		bcd_to_integer(m_regs[1]),      // minute
		bcd_to_integer(m_regs[0]));     // second
}


//**************************************************************************
//  JRC 6355E DEVICE
//**************************************************************************

// device type definition
const device_type JRC6355E = &device_creator<jrc6355e_device>;


//-------------------------------------------------
//  jrc6355e_device - constructor
//-------------------------------------------------

jrc6355e_device::jrc6355e_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: rtc4543_device(mconfig, JRC6355E, "JRC 6355E RTC", tag, owner, clock, "jrc6355e", __FILE__)
{
}


//-------------------------------------------------
//  ce_rising - CE rising edge trigger
//-------------------------------------------------

void jrc6355e_device::ce_rising()
{
	m_curbit = 0; // force immediate reload of output data
	load_bit(6);
}



//-------------------------------------------------
//  ce_falling - CE falling edge trigger
//-------------------------------------------------

void jrc6355e_device::ce_falling()
{
	// update occurs on falling edge of CE after minutes are written
	if (m_wr && m_curbit >= 48)
	{
		// seconds are zeroed
		m_regs[0] = 0;
		update_effective();
	}
}


//-------------------------------------------------
//  clk_rising - CLK rising edge trigger
//-------------------------------------------------

void jrc6355e_device::clk_rising()
{
	if (m_curbit == 56)
		return;

	if (m_wr)
		store_bit(6 - (m_curbit / 8));
}


//-------------------------------------------------
//  clk_falling - CLK falling edge trigger
//-------------------------------------------------

void jrc6355e_device::clk_falling()
{
	if (m_curbit == 56)
		return;

	advance_bit();

	if (!m_wr && m_curbit != 56)
		load_bit(6 - (m_curbit / 8));
}
