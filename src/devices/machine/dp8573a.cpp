// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

/*
 * DP8572A/DP8573A Real Time Clock (RTC)
 *
 * The DP8572A has several enhancements over the DP8573A:
 *  - additional page of 32 bytes of RAM
 *  - programmable prescaler (allows choice of 32.768KHz, 4.194304MHz, 4.9152MHz
 *    and 32.0KHz oscillators)
 *  - power fail delay
 *
 * TODO:
 *  - dp8572a programmable prescaler
 */

#include "emu.h"
#include "machine/dp8573a.h"

#include "machine/timehelp.h"

#include <algorithm>
#include <iterator>

#define LOG_TICKS   (1U << 1)
#define LOG_ALL     (LOG_GENERAL | LOG_TICKS)

#define VERBOSE (0)
#include "logmacro.h"

enum : uint8_t
{
	REG_MSR             = 0x00, // Main Status Register
	REG_RTMR            = 0x01, // Not Applicable / Real-Time Mode Register
	REG_OMR             = 0x02, // Not Applicable / Output Mode Register
	REG_PFR_ICR0        = 0x03, // Periodic Flag Register / Interrupt Control Register 0
	REG_TSCR_ICR1       = 0x04, // Time Save Control Register / Interrupt Control Register 1
	REG_HUNDREDTH       = 0x05, // Hundredths and Tenths of a Second (0-99)
	REG_SECOND          = 0x06, // Seconds (0-59)
	REG_MINUTE          = 0x07, // Minutes (0-59)
	REG_HOUR            = 0x08, // Hours (1-12, 0-23)
	REG_DAY             = 0x09, // Day of Month (1-28/29/30/31)
	REG_MONTH           = 0x0a, // Month (1-12)
	REG_YEAR            = 0x0b, // Year (0-99)
	// 0x0c - RAM
	REG_RAM_D1D0        = 0x0d, // RAM, D1/D0 bits only
	REG_DAYOFWEEK       = 0x0e, // Day of Week (1-7)
	REG_NA_0FH          = 0x0f,
	REG_NA_10H          = 0x10,
	REG_NA_11H          = 0x11,
	REG_NA_12H          = 0x12,
	REG_COMP_SECOND     = 0x13, // Seconds Compare RAM (0-59)
	REG_COMP_MINUTE     = 0x14, // Minutes Compare RAM (0-59)
	REG_COMP_HOUR       = 0x15, // Hours Compare RAM (1-12, 0-23)
	REG_COMP_DAY        = 0x16, // Day of Month Compare RAM (1-28/29/30/31)
	REG_COMP_MONTH      = 0x17, // Month Compare RAM (1-12)
	REG_COMP_DAYOFWEEK  = 0x18, // Day of Week Compare RAM (1-7)
	REG_SAVE_SECOND     = 0x19, // Seconds Time Save RAM
	REG_SAVE_MINUTE     = 0x1a, // Minutes Time Save RAM
	REG_SAVE_HOUR       = 0x1b, // Hours Time Save RAM
	REG_SAVE_DAY        = 0x1c, // Day of Month Time Save RAM
	REG_SAVE_MONTH      = 0x1d, // Month Time Save RAM
	// 0x1e - RAM
	REG_TEST            = 0x1f, // RAM / Test Mode Register

	MSR_INT             = 0x01, // Interrupt Status
	MSR_PF              = 0x02, // Power Fail Interrupt
	MSR_PER             = 0x04, // Period Interrupt
	MSR_AL              = 0x08, // Alarm Interrupt
	MSR_RS              = 0x40, // Register Select Bit
	MSR_PS              = 0x80, // Page Select (DP8572A)
	MSR_RAM_MASK        = 0xf0,
	MSR_INT_MASK        = 0x0e,
	MSR_CLEARABLE_MASK  = 0x0c,

	PFR_1MIN            = 0x01, // Minutes flag
	PFR_10S             = 0x02, // 10-second flag
	PFR_1S              = 0x04, // Seconds flag
	PFR_100MS           = 0x08, // 100-millisecond flag
	PFR_10MS            = 0x10, // 10-millisecond flag
	PFR_1MS             = 0x20, // Millisecond flag
	PFR_OSF             = 0x40, // Oscillator Failed / Single Supply Bit
	PFR_TM              = 0x80, // Test Mode Enable
	PFR_READ_CLEAR_MASK = 0x3f,

	TSCR_RAM_MASK       = 0x3f,
	TSCR_PFDE           = 0x20, // Power Fail Delay Enable (DP8572A)
	TSCR_NA             = 0x40, // N/A
	TSCR_LBF            = 0x40, // Low Battery Flag (DP8572A)
	TSCR_TS             = 0x80, // Time Save Enable

	RTMR_LY0            = 0x01, // Leap Year LSB
	RTMR_LY1            = 0x02, // Leap Year MSB
	RTMR_LY             = 0x03,
	RTMR_12H            = 0x04, // 12/!24 hour mode
	RTMR_CSS            = 0x08, // Clock Start/!Stop
	RTMR_IPF            = 0x10, // Interrupt PF Operation
	RTMR_XT0            = 0x40, // Crystal Freq. XT0 (DP8572A)
	RTMR_XT1            = 0x80, // Crystal Freq. XT1 (DP8572A)
	RTMR_RAM_MASK       = 0xe0,

	OMR_RAM_MASK        = 0x7f,
	OMR_MO              = 0x80, // MFO Pin as Oscillator

	ICR0_MN             = 0x01, // Minutes enable
	ICR0_TS             = 0x02, // 10-second enable
	ICR0_S              = 0x04, // Seconds enable
	ICR0_HM             = 0x08, // 100 millisecond enable
	ICR0_TM             = 0x10, // 10 millisecond enable
	ICR0_1M             = 0x20, // Milliseconds enable
	ICR0_RAM_MASK       = 0xc0,

	ICR1_SC             = 0x01, // Second compare enable
	ICR1_MN             = 0x02, // Minute compare enable
	ICR1_HR             = 0x04, // Hour compare enable
	ICR1_DOM            = 0x08, // Day of month compare enable
	ICR1_MO             = 0x10, // Month compare enable
	ICR1_DOW            = 0x20, // Day of week compare enable
	ICR1_ALE            = 0x40, // Alarm interrupt enable
	ICR1_PFE            = 0x80, // Power fail interrupt enable
	ICR1_COMPARE_MASK   = 0x3f
};

// device type definition
DEFINE_DEVICE_TYPE(DP8572A, dp8572a_device, "dp8572a", "DP8572A Real Time Clock")
DEFINE_DEVICE_TYPE(DP8573A, dp8573a_device, "dp8573a", "DP8573A Real Time Clock")

dp8573a_device::dp8573a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, device_rtc_interface(mconfig, *this)
	, m_intr_cb(*this)
	, m_mfo_cb(*this)
{
}

dp8573a_device::dp8573a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dp8573a_device(mconfig, DP8573A, tag, owner, clock)
{
}

dp8572a_device::dp8572a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dp8573a_device(mconfig, DP8572A, tag, owner, clock)
{
}

void dp8573a_device::device_start()
{
	m_ram = std::make_unique<uint8_t[]>(ram_size());

	save_pointer(NAME(m_ram), ram_size());
	save_item(NAME(m_tscr));
	save_item(NAME(m_pfr));
	save_item(NAME(m_millis));

	m_timer = timer_alloc(FUNC(dp8573a_device::msec_tick), this);
	m_timer->adjust(attotime::never);

	m_tscr = 0;

	m_timer->adjust(attotime::from_msec(1), 0, attotime::from_msec(1));
}

void dp8573a_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_millis = 0;
	m_ram[REG_HUNDREDTH] = 0;
	m_ram[REG_SECOND] = time_helper::make_bcd(second);
	m_ram[REG_MINUTE] = time_helper::make_bcd(minute);
	m_ram[REG_HOUR] = time_helper::make_bcd(hour);
	m_ram[REG_DAY] = time_helper::make_bcd(day);
	m_ram[REG_MONTH] = time_helper::make_bcd(month);
	m_ram[REG_YEAR] = time_helper::make_bcd(year);
	m_ram[REG_DAYOFWEEK] = time_helper::make_bcd(day_of_week);

	m_pfr = 0;

	// FIXME: should probably rely on nvram start/stop state
	m_ram[REG_RTMR] = RTMR_CSS;
}

void dp8573a_device::save_registers()
{
	m_ram[REG_SAVE_SECOND] = m_ram[REG_SECOND];
	m_ram[REG_SAVE_MINUTE] = m_ram[REG_MINUTE];
	m_ram[REG_SAVE_HOUR]   = m_ram[REG_HOUR];
	m_ram[REG_SAVE_DAY]    = m_ram[REG_DAY];
	m_ram[REG_SAVE_MONTH]  = m_ram[REG_MONTH];
}

TIMER_CALLBACK_MEMBER(dp8573a_device::msec_tick)
{
	if ((m_pfr & PFR_OSF) || !(m_ram[REG_RTMR] & RTMR_CSS))
	{
		LOGMASKED(LOG_TICKS, "Tick suppressed due to OSF or !CSS\n");
		return;
	}

	m_pfr |= PFR_1MS;

	bool carry = false;
	bool tens_carry = false;
	time_helper::inc_bcd(&m_millis, 0xff, 0x00, 0x09, &tens_carry);
	if (tens_carry)
	{
		m_pfr |= PFR_10MS;
		carry = time_helper::inc_bcd(&m_ram[REG_HUNDREDTH], 0xff, 0x00, 0x99, &tens_carry);
		if (tens_carry)
			m_pfr |= PFR_100MS;
	}
	if (carry)
	{
		m_pfr |= PFR_1S;
		carry = time_helper::inc_bcd(&m_ram[REG_SECOND], 0xff, 0x00, 0x59, &tens_carry);
		if (tens_carry)
			m_pfr |= PFR_10S;
	}
	if (carry)
	{
		m_pfr |= PFR_1MIN;
		carry = time_helper::inc_bcd(&m_ram[REG_MINUTE], 0xff, 0x00, 0x59);
	}
	if (carry)
	{
		if (m_ram[REG_RTMR] & RTMR_12H)
		{
			carry = time_helper::inc_bcd(&m_ram[REG_HOUR], 0xff, 0x01, 0x12);
			if (carry)
			{
				m_ram[REG_HOUR] |= 0x20;
				carry = !(m_ram[REG_HOUR] & 0x20);
			}
		}
		else
		{
			carry = time_helper::inc_bcd(&m_ram[REG_HOUR], 0xff, 0x00, 0x23);
		}
	}
	if (carry)
	{
		static const uint8_t daysinmonth[] = { 0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31 };

		time_helper::inc_bcd(&m_ram[REG_DAYOFWEEK], 0xff, 0x01, 0x07);

		uint8_t month = time_helper::from_bcd(m_ram[REG_MONTH]);

		uint8_t maxdays;
		if (month == 2 && (m_ram[REG_RTMR] & RTMR_LY) == 0)
		{
			maxdays = 0x29;
		}
		else if (month >= 1 && month <= 12)
		{
			maxdays = daysinmonth[month - 1];
		}
		else
		{
			maxdays = 0x31;
		}

		carry = time_helper::inc_bcd(&m_ram[REG_DAY], 0xff, 0x01, maxdays);
	}
	if (carry)
	{
		carry = time_helper::inc_bcd(&m_ram[REG_MONTH], 0xff, 0x01, 0x12);
	}
	if (carry)
	{
		carry = time_helper::inc_bcd(&m_ram[REG_YEAR], 0xff, 0x00, 0x99);
	}
	if (carry)
	{
		// Advance the leap-year counter
		uint8_t leap = m_ram[REG_RTMR] & RTMR_LY;
		leap = (leap + 1) & RTMR_LY;
		m_ram[REG_RTMR] &= ~RTMR_LY;
		m_ram[REG_RTMR] |= leap;
	}

	// Check for Time Save mode
	if (m_tscr & TSCR_TS)
	{
		save_registers();
	}

	// Check for periodic interrupts
	const uint8_t icr0 = m_ram[REG_PFR_ICR0] & ~ICR0_RAM_MASK;
	const uint8_t pfr = m_pfr & ~ICR0_RAM_MASK;
	if (icr0 & pfr)
	{
		set_interrupt(MSR_PER);
	}

	const uint8_t icr1 = m_ram[REG_TSCR_ICR1] & ICR1_COMPARE_MASK;
	if (icr1)
	{
		if (m_ram[REG_SECOND] == m_ram[REG_COMP_SECOND] ||
			m_ram[REG_MINUTE] == m_ram[REG_COMP_MINUTE] ||
			m_ram[REG_HOUR] == m_ram[REG_COMP_HOUR] ||
			m_ram[REG_DAY] == m_ram[REG_COMP_DAY] ||
			m_ram[REG_MONTH] == m_ram[REG_COMP_MONTH] ||
			m_ram[REG_DAYOFWEEK] == m_ram[REG_COMP_DAYOFWEEK])
		{
			set_interrupt(MSR_AL);
		}
	}
}

void dp8573a_device::set_interrupt(uint8_t mask)
{
	bool was_intr = m_ram[REG_MSR] & MSR_INT;
	m_ram[REG_MSR] |= mask;

	if (m_ram[REG_MSR] & MSR_INT_MASK)
		m_ram[REG_MSR] |= MSR_INT;

	if (!was_intr && (m_ram[REG_MSR] & MSR_INT))
		m_intr_cb(0);
}

void dp8573a_device::clear_interrupt(uint8_t mask)
{
	bool was_intr = m_ram[REG_MSR] & MSR_INT;
	m_ram[REG_MSR] &= ~mask;

	if (was_intr && !(m_ram[REG_MSR] & MSR_INT))
		m_intr_cb(1);
}

void dp8573a_device::write(offs_t offset, uint8_t data)
{
	LOG("%s: Register Write: %02x = %02x\n", machine().describe_context(), offset, data);

	switch (offset)
	{
		case REG_MSR: // Main Status Register
			m_ram[offset] &= ~MSR_RAM_MASK;
			m_ram[offset] |= data & MSR_RAM_MASK;
			if (data & MSR_CLEARABLE_MASK)
				clear_interrupt(data & MSR_CLEARABLE_MASK);
			break;

		case REG_RTMR: // Not Applicable / Real-Time Mode Register
			if (m_ram[REG_MSR] & MSR_RS)
			{
				const uint8_t old = m_ram[offset];
				m_ram[offset] = data;
				if ((old ^ data) & RTMR_12H)
				{
					uint8_t hour;
					if (old & RTMR_12H)
						hour = time_helper::from_bcd(m_ram[REG_HOUR] & 0x1f) + (BIT(m_ram[REG_HOUR], 5) ? 12 : 0);
					else
						hour = time_helper::from_bcd(m_ram[REG_HOUR]);

					if (data & RTMR_12H)
					{
						m_ram[REG_HOUR] = time_helper::make_bcd(hour % 12);
						m_ram[REG_HOUR] |= (hour > 11) ? 0x20 : 0;
					}
					else
					{
						m_ram[REG_HOUR] = time_helper::make_bcd(hour);
					}
				}
			}
			break;

		case REG_OMR: // Not Applicable / Output Mode Register
			if (m_ram[REG_MSR] & MSR_RS)
			{
				// Not yet implemented: Buffered Crystal Oscillator output on MFO pin
				m_ram[offset] = data;
			}
			break;

		case REG_PFR_ICR0: // Periodic Flag Register / Interrupt Control Register 0
			if (m_ram[REG_MSR] & MSR_RS)
			{
				m_ram[offset] = data;
			}
			else
			{
				m_pfr &= ~PFR_TM;
				m_pfr |= data & PFR_TM;
			}
			break;

		case REG_TSCR_ICR1: // Time Save Control Register / Interrupt Control Register 1
			if (m_ram[REG_MSR] & MSR_RS)
			{
				m_ram[offset] = data;
			}
			else
			{
				m_tscr = data & ~TSCR_NA;
				if (data & TSCR_TS)
					save_registers();
			}
			break;

		case REG_RAM_D1D0: // RAM, D1/D0 bits only
			m_ram[offset] = data & 3;
			break;

		case REG_NA_0FH:
		case REG_NA_10H:
		case REG_NA_11H:
		case REG_NA_12H:
			break;

		default:
			m_ram[offset] = data;
			break;
	}
}

uint8_t dp8573a_device::read(offs_t offset)
{
	uint8_t ret = m_ram[offset];

	if (offset >= REG_RTMR && offset <= REG_TSCR_ICR1)
	{
		if (m_ram[REG_MSR] & MSR_RS)
		{
		}
		else
		{
			switch (offset)
			{
				case REG_RTMR:
				case REG_OMR:
					ret = 0;
					break;
				case REG_PFR_ICR0:
					ret = m_pfr;
					m_pfr &= ~PFR_READ_CLEAR_MASK;
					break;
				case REG_TSCR_ICR1:
					ret = m_tscr;
					break;
				default:
					break;
			}
		}
	}

	LOG("%s: Register Read: %02x = %02x\n", machine().describe_context(), offset, ret);
	return ret;
}

void dp8573a_device::nvram_default()
{
	std::fill_n(m_ram.get(), ram_size(), 0);
}

bool dp8573a_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = util::read(file, m_ram.get(), ram_size());
	if (err || (actual != ram_size()))
		return false;

	return true;
}

bool dp8573a_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = util::write(file, m_ram.get(), ram_size());
	return !err;
}

void dp8572a_device::write(offs_t offset, uint8_t data)
{
	if (offset && (m_ram[REG_MSR] & MSR_PS))
		m_ram[offset + 32] = data;
	else
		dp8573a_device::write(offset, data);
}

uint8_t dp8572a_device::read(offs_t offset)
{
	if (offset && (m_ram[REG_MSR] & MSR_PS))
		return m_ram[offset + 32];
	else
		return dp8573a_device::read(offset);
}
