// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    DP8573 Real Time Clock (RTC)

***************************************************************************/

#include "emu.h"
#include "machine/dp8573.h"
#include "machine/timehelp.h"

#define LOG_GENERAL (1 << 0)
#define LOG_TICKS   (1 << 1)
#define LOG_ALL     (LOG_GENERAL | LOG_TICKS)

#define VERBOSE (0)
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(DP8573, dp8573_device, "dp8573", "DP8573 Real-Time Clock")

dp8573_device::dp8573_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DP8573, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, m_intr_cb(*this)
	, m_mfo_cb(*this)
{
}

void dp8573_device::device_start()
{
	save_item(NAME(m_ram));
	save_item(NAME(m_tscr));
	save_item(NAME(m_pfr));
	save_item(NAME(m_millis));

	m_timer = timer_alloc(TIMER_ID);
	m_timer->adjust(attotime::never);

	m_intr_cb.resolve_safe();
	m_mfo_cb.resolve_safe();

	memset(m_ram, 0, 32);
	sync_time();

	m_tscr = 0;

	m_timer->adjust(attotime::from_msec(1), 0, attotime::from_msec(1));
}

void dp8573_device::sync_time()
{
	system_time systime;
	machine().base_datetime(systime);

	m_millis = 0;
	m_ram[REG_HUNDREDTH] = 0;
	m_ram[REG_SECOND] = time_helper::make_bcd(systime.utc_time.second);
	m_ram[REG_MINUTE] = time_helper::make_bcd(systime.utc_time.minute);
	m_ram[REG_HOUR] = time_helper::make_bcd(systime.utc_time.hour);
	m_ram[REG_DAY] = time_helper::make_bcd(systime.utc_time.mday);
	m_ram[REG_MONTH] = time_helper::make_bcd(systime.utc_time.month + 1);
	m_ram[REG_YEAR] = time_helper::make_bcd(systime.utc_time.year % 100);
	m_ram[REG_DAYOFWEEK] = time_helper::make_bcd(systime.utc_time.weekday + 1);

	m_pfr = 0;

	// FIXME: should probably rely on nvram start/stop state
	m_ram[REG_RTMR] = RTMR_CSS;
}

void dp8573_device::save_registers()
{
	m_ram[REG_SAVE_SECOND] = m_ram[REG_SECOND];
	m_ram[REG_SAVE_MINUTE] = m_ram[REG_MINUTE];
	m_ram[REG_SAVE_HOUR]   = m_ram[REG_HOUR];
	m_ram[REG_SAVE_DAY]    = m_ram[REG_DAY];
	m_ram[REG_SAVE_MONTH]  = m_ram[REG_MONTH];
}

void dp8573_device::device_timer(emu_timer &timer, device_timer_id id, int param)
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

void dp8573_device::set_interrupt(uint8_t mask)
{
	bool was_intr = m_ram[REG_MSR] & MSR_INT;
	m_ram[REG_MSR] |= mask;

	if (m_ram[REG_MSR] & MSR_INT_MASK)
		m_ram[REG_MSR] |= MSR_INT;

	if (!was_intr && (m_ram[REG_MSR] & MSR_INT))
		m_intr_cb(0);
}

void dp8573_device::clear_interrupt(uint8_t mask)
{
	bool was_intr = m_ram[REG_MSR] & MSR_INT;
	m_ram[REG_MSR] &= ~mask;

	if (was_intr && !(m_ram[REG_MSR] & MSR_INT))
		m_intr_cb(1);
}

void dp8573_device::write(offs_t offset, u8 data)
{
	LOGMASKED(LOG_GENERAL, "%s: DP8573 - Register Write: %02x = %02x\n", machine().describe_context(), offset, data);

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

u8 dp8573_device::read(offs_t offset)
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

	LOGMASKED(LOG_GENERAL, "%s: DP8573 - Register Read: %02x = %02x\n", machine().describe_context(), offset, ret);
	return ret;
}

void dp8573_device::nvram_default()
{
	memset(m_ram, 0, 32);
	sync_time();
}

bool dp8573_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	if (file.read(m_ram, 32, actual) || actual != 32)
		return false;

	sync_time();
	return true;
}

bool dp8573_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(m_ram, 32, actual) && actual == 32;
}
