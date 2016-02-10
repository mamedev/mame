// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    rtc4543.c - Epson R4543 real-time clock chip emulation
    by R. Belmont

    TODO: writing (not done by System 12 or 23 so no test case)

**********************************************************************/

#include "rtc4543.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define VERBOSE 0

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type RTC4543 = &device_creator<rtc4543_device>;


//-------------------------------------------------
//  rtc4543_device - constructor
//-------------------------------------------------

rtc4543_device::rtc4543_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, RTC4543, "R4543 RTC", tag, owner, clock, "rtc4543", __FILE__),
		device_rtc_interface(mconfig, *this),
		data_cb(*this), m_ce(0), m_clk(0), m_wr(0), m_data(0), m_shiftreg(0), m_curreg(0), m_curbit(0), m_clock_timer(nullptr)
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
	save_item(NAME(m_shiftreg));
	save_item(NAME(m_regs));
	save_item(NAME(m_curreg));
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
	m_shiftreg = 0;
	m_curreg = 0;
	m_curbit = 0;
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void rtc4543_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	advance_seconds();
}


static inline UINT8 make_bcd(UINT8 data)
{
	return ((data / 10) << 4) | (data % 10);
}

//-------------------------------------------------
//  rtc_clock_updated -
//-------------------------------------------------

void rtc4543_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	static const int weekday[7] = { 7, 1, 2, 3, 4, 5, 6 };

	m_regs[0] = make_bcd(second);                   // seconds (BCD, 0-59) in bits 0-6, bit 7 = battery low
	m_regs[1] = make_bcd(minute);                   // minutes (BCD, 0-59)
	m_regs[2] = make_bcd(hour);                     // hour (BCD, 0-23)
	m_regs[3] = make_bcd(weekday[day_of_week - 1]); // low nibble = day of the week
	m_regs[3] |= (make_bcd(day) & 0x0f) << 4;       // high nibble = low digit of day
	m_regs[4] = (make_bcd(day) >> 4);               // low nibble = high digit of day
	m_regs[4] |= (make_bcd(month & 0x0f) << 4);     // high nibble = low digit of month
	m_regs[5] = make_bcd(month & 0x0f) >> 4;        // low nibble = high digit of month
	m_regs[5] |= (make_bcd(year % 10) << 4);        // high nibble = low digit of year
	m_regs[6] = make_bcd(year % 100) >> 4;          // low nibble = tens digit of year (BCD, 0-9)
}

//-------------------------------------------------
//  ce_w - chip enable write
//-------------------------------------------------

WRITE_LINE_MEMBER( rtc4543_device::ce_w )
{
	if (VERBOSE) printf("RTC4543 '%s' CE: %u\n", tag(), state);

	if (!state && m_ce) // complete transfer
	{
	}
	else if (state && !m_ce) // start new data transfer
	{
		m_curreg = 0;
		m_curbit = 0; // force immediate reload of output data
	}

	m_ce = state;
}

//-------------------------------------------------
//  wr_w - data direction line write
//-------------------------------------------------

WRITE_LINE_MEMBER( rtc4543_device::wr_w )
{
	if (VERBOSE) logerror("RTC4543 '%s' WR: %u\n", tag(), state);

	m_wr = state;
}

//-------------------------------------------------
//  clk_w - serial clock write
//-------------------------------------------------

WRITE_LINE_MEMBER( rtc4543_device::clk_w )
{
	if (VERBOSE) logerror("RTC4543 '%s' CLK: %u\n", tag(), state);

	if (!m_ce) return;

	// rising edge - read data becomes valid here
	if (!m_clk && state)
	{
		if (!m_wr)
		{
			// reload data?
			if ((m_curbit & 7) == 0)
			{
				m_shiftreg = m_regs[m_curreg++];

				if (VERBOSE)
					logerror("RTC4543 '%s' sending byte: %02x\n", tag(), m_shiftreg);
			}

			// shift data bit
			// note: output data does not change when clk at final bit
			if (m_curbit != 55)
			{
				m_data = m_shiftreg & 1;
				m_curbit++;
				m_shiftreg >>= 1;
				data_cb(m_data);
			}
		}
	}

	m_clk = state;
}


//-------------------------------------------------
//  data_w - I/O write
//-------------------------------------------------

WRITE_LINE_MEMBER( rtc4543_device::data_w )
{
	if (VERBOSE) logerror("RTC4543 '%s' I/O: %u\n", tag(), state);

	m_data = state & 1;
}


//-------------------------------------------------
//  data_r - I/O read
//-------------------------------------------------

READ_LINE_MEMBER( rtc4543_device::data_r )
{
	return m_data;
}
