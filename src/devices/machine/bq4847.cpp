// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*
    Texas Instruments/Benchmarq BQ4847 Real-time clock

    Although featuring a similar interface, this chip is sufficiently
    different from the BQ4842/BQ4852 that a separate implementation
    makes sense.

    This chip is functionally equivalent to the BQ4845; it does not support
    a backup battery. Most datasheets about the BQ4847 are incomplete and
    refer to the BQ4845.

    Supports 24h/12h and Daylight saving
    Supports leap years

    No internal memory, only clock registers

    Michael Zapf, April 2020
*/
#include "emu.h"
#include "bq4847.h"

#define LOG_WARN         (1U<<1)    // Warnings
#define LOG_CLOCK        (1U<<2)    // Clock operation
#define LOG_REG          (1U<<3)    // Register write
#define LOG_WATCHDOG     (1U<<4)    // Watchdog
#define LOG_TRANSFER     (1U<<5)    // Transfer

#define VERBOSE ( LOG_GENERAL | LOG_WARN )
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(BQ4845, bq4845_device, "bq4845", "Benchmarq BQ4845 RTC")
DEFINE_DEVICE_TYPE(BQ4847, bq4847_device, "bq4847", "Benchmarq BQ4847 RTC")

enum
{
	reg_seconds = 0,        // 0x00 - 0x59
	reg_alarmseconds,       // 0xc0 to ignore
	reg_minutes,            // 0x00 - 0x59
	reg_alarmminutes,       // 0xc0 to ignore
	reg_hours,              // 0x00 - 0x23 (24h) or 0x01-0x12 (AM), 0x81-0x92 (PM)
	reg_alarmhours,         // 0xc0 to ignore
	reg_date,               // 0x01 - 0x31
	reg_alarmdate,          // 0xc0 to ignore
	reg_days,               // 0x01 (sun) - 0x07 (sat)
	reg_month,              // 0x01 - 0x12
	reg_year,               // 0x00 - 0x99
	reg_rates,              // 0 [--WD--] [-------RS---------]
	reg_interrupts,         // 0  0  0  0 AIE  PIE   PWRIE ABE    0x00 on powerup
	reg_flags,              // 0  0  0  0  AF   PF    PWRF BVF    0x00 after reading
	reg_control,            // 0  0  0  0 UTI STOP* 24/12* DSE
	reg_unused              // 0x00
};

enum
{
	INTERRUPT_AIE = 0x08,
	INTERRUPT_PIE = 0x04,
	INTERRUPT_PWRIE = 0x02,
	INTERRUPT_ABE = 0x01,
	FLAG_AF = 0x08,
	FLAG_PF = 0x04,
	FLAG_PWRF = 0x02,
	FLAG_BVF = 0x01,
	CONTROL_UTI = 0x08,
	CONTROL_STOP = 0x04,
	CONTROL_24 = 0x02,
	CONTROL_DSE = 0x01
};

//-------------------------------------------------
//  Constructors for basetype
//-------------------------------------------------

bq4847_device::bq4847_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	device_nvram_interface(mconfig, *this),
	device_rtc_interface(mconfig, *this),
	m_region(*this, DEVICE_SELF),
	m_wdo_handler(*this),
	m_int_handler(*this),
	m_rst_handler(*this),
	m_periodic_timer(nullptr),
	m_watchdog_timer(nullptr),
	m_wdo_state(1),
	m_int_state(1),
	m_rst_state(1),
	m_wdi_state(-1),
	m_writing(false)
{
}

bq4847_device::bq4847_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock)
	: bq4847_device(mconfig, BQ4847, tag, owner, clock)
{
}

bq4845_device::bq4845_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock)
	: bq4847_device(mconfig, BQ4845, tag, owner, clock)
{
}

// device_rtc_interface

void bq4847_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	if ((m_register[reg_control] & CONTROL_STOP) != 0)
	{
		m_register[reg_hours] = ((m_register[reg_control] & CONTROL_24) != 0) ? convert_to_bcd(hour) :
			(((hour % 24) >= 12) ? 0x80 : 0x00) | convert_to_bcd((hour % 12) ? (hour % 12) : 12);
		m_register[reg_minutes] = convert_to_bcd(minute);
		m_register[reg_seconds] = convert_to_bcd(second);
		m_register[reg_year] = convert_to_bcd(year);
		m_register[reg_month] = convert_to_bcd(month);
		m_register[reg_date] = convert_to_bcd(day);
		m_register[reg_days] = convert_to_bcd(day_of_week);
	}

	// Clear the saved flags (TODO: check that flags set before power down, or during battery backup are lost)
	m_register[reg_flags] = 0x00;

	// Interrupts must be re-enabled on power-up (TODO: check, datasheet does not explicitly say ABE & PIE are cleared)
	m_register[reg_interrupts] = 0x00;

	// TODO: check if user buffer is battery backed
	// TODO: What if UTI is set?
	std::copy_n(m_register, std::size(m_register), m_userbuffer);

	// What about the DSE flag?
}

bool bq4847_device::increment_bcd(uint8_t& bcdnumber, uint8_t limit, uint8_t min)
{
	if (bcdnumber >= limit)
	{
		bcdnumber = min;
		return true;
	}
	else
	{
		uint8_t dig0 = bcdnumber & 0x0f;
		uint8_t dig1 = bcdnumber & 0xf0;

		if (dig0 == 9)
		{
			bcdnumber = dig1 + 0x10;
		}
		else bcdnumber++;
	}
	return false;
}

/*
    Update cycle, called every second
    The BQ RTCs use BCD representation

    TODO: We may not be able to use the parent class advance methods, since we
    have to work with BCD (even with invalid values). Check this.
*/
TIMER_CALLBACK_MEMBER(bq4847_device::update_callback)
{
	// Just for debugging
	static const char* dow[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

	bool carry = true;
	bool newsec = false;

	if (carry)
	{
		carry = increment_bcd(m_register[reg_seconds], 0x59, 0);
		newsec = true;
	}

	if (carry)
		carry = increment_bcd(m_register[reg_minutes], 0x59, 0);

	if (carry)
		carry = advance_hours_bcd();

	if (carry)
		advance_days_bcd();

	LOGMASKED(LOG_CLOCK, "%s 20%02x-%02x-%02x %02x:%02x:%02x\n",
		dow[m_register[reg_days] - 1], m_register[reg_year], m_register[reg_month], m_register[reg_date],
		m_register[reg_hours], m_register[reg_minutes], m_register[reg_seconds]);

	if (newsec)
	{
		if ((m_register[reg_control] & CONTROL_UTI) == 0)
		{
			LOGMASKED(LOG_TRANSFER, "Transfer to external regs\n");
			for (int i = reg_seconds; i < reg_unused; i++)
			{
				if (is_clock_register(i)) m_userbuffer[i] = m_register[i];
			}
		}

		if (check_alarm(reg_date, reg_alarmdate) &&
			check_alarm(reg_hours, reg_alarmhours) &&
			check_alarm(reg_minutes, reg_alarmminutes) &&
			check_alarm(reg_seconds, reg_alarmseconds))
		{
			m_userbuffer[reg_flags] |= FLAG_AF;
			update_int();
		}
	}
}

bool bq4847_device::advance_hours_bcd()
{
	bool carry = false;
	// Handle DST
	if ((m_register[reg_control] & CONTROL_DSE) != 0
		&& (m_register[reg_month] == 4) && (m_register[reg_days] == 0) && (m_register[reg_date] < 8)  // first Sunday in April
		&& (m_register[reg_hours] == 0x01))
		m_register[reg_hours] = 0x03;
	else
	{
		// Increment hour unless the DSE bit is set and we are at 1:59 on the last Sunday in October
		if ((m_register[reg_control] & CONTROL_DSE) == 0
			|| (m_register[reg_month] != 10) || (m_register[reg_days] != 0) || (m_register[reg_date] <= 23)  // last Sunday in October
			|| (m_register[reg_hours] != 0x01))
		{
			if ((m_register[reg_control] & CONTROL_24) != 0)
			{
				// 24h:  0->1->...->23->0(+1)
				increment_bcd(m_register[reg_hours], 0xff, 0);
				if (m_register[reg_hours] == 0x24)
				{
					m_register[reg_hours] = 0;
					carry = true;
				}
			}
			else
			{
				// 12h:  12->1->2->...->11->12'->1'->...->11'->12(+1)
				increment_bcd(m_register[reg_hours], 0xff, 0);
				switch (m_register[reg_hours])
				{
				case 0x12:
					m_register[reg_hours] = 0x92;  // 11:59 am -> 12:00 pm
					break;
				case 0x93:
					m_register[reg_hours] = 0x81;  // 12:59 pm -> 01:00 pm
					break;
				case 0x92:
					m_register[reg_hours] = 0x12;  // 11:59 pm -> 12:00 am
					carry = true;
					break;
				case 0x13:
					m_register[reg_hours] = 0x01;  // 12:59 am -> 01:00 am
					break;
				}
			}
		}
	}
	return carry;
}

void bq4847_device::advance_days_bcd()
{
	bool carry = false;

	// BCD-encoded numbers
	static const int days_in_month_table[12] =
	{
		0x31, 0x28, 0x31, 0x30, 0x31, 0x30,
		0x31, 0x31, 0x30, 0x31, 0x30, 0x31
	};

	uint8_t month = bcd_to_integer(m_register[reg_month]);
	if (month > 12) month = 12;

	uint8_t days = days_in_month_table[month - 1];

	// Leap years are indeed handled (but the year is only 2-digit)
	if ((month == 2) && ((bcd_to_integer(m_register[reg_year]) % 4) == 0))
		days = 0x29;

	increment_bcd(m_register[reg_days], 7, 1);  // Increment the day-of-week (without carry)
	carry = increment_bcd(m_register[reg_date], days, 1);

	if (carry)
	{
		increment_bcd(m_register[reg_month], 0xff, 1);
		if (m_register[reg_month] == 0x13)
		{
			m_register[reg_month] = 0x01;
			increment_bcd(m_register[reg_year], 0xff, 0);
		}
	}
}

bool bq4847_device::check_alarm(int now, int alarm)
{
	return (m_register[alarm] & 0xc0) == 0xc0 || (m_register[alarm] == m_register[now]);
}

uint8_t bq4847_device::read(offs_t address)
{
	int regnum = address & 0x0f;
	uint8_t value = m_userbuffer[regnum];

	if (regnum == reg_flags)
	{
		value &= 0x7f;
		m_userbuffer[reg_flags] = 0x00;
		update_int();
	}
	else if (regnum >= reg_interrupts && regnum <= reg_control)
		value &= 0xf;
	else if (regnum == reg_unused)
		value = 0;  // Reg 15 is locked to 0 in BQ4847

	LOGMASKED(LOG_REG, "Reg %d -> %02x\n", regnum, value);

	return value;
}

void bq4847_device::write(offs_t address, uint8_t data)
{
	int regnum = address & 0x0f;

	LOGMASKED(LOG_REG, "Reg %d <- %02x\n", regnum, data);

	if (regnum == reg_flags)
	{
		LOGMASKED(LOG_WARN, "Ignoring write attempt to flag bit register (%02x)\n", data);
		return;
	}

	bool uti_set = (m_register[reg_control] & CONTROL_UTI) != 0;

	m_userbuffer[regnum] = data;

	// If inhibit is not set, any write to the time/date registers
	// is immediately set
	if (uti_set && is_clock_register(regnum))
		m_writing = true;
	else
		m_register[regnum] = m_userbuffer[regnum];

	if (regnum == reg_rates)
	{
		set_watchdog_timer();
		set_periodic_timer();
	}
	else if (regnum == reg_control)
	{
		bool uti_set_now = (m_register[reg_control] & CONTROL_UTI) != 0;
		LOGMASKED(LOG_TRANSFER, "Update transfer %s\n", uti_set_now ? "inhibit" : "enable");

		// After we have written to the registers, transfer to the internal regs
		if (uti_set && !uti_set_now && m_writing)
		{
			LOGMASKED(LOG_TRANSFER, "Transfer to internal regs\n");
			for (int i = reg_seconds; i < reg_unused; i++)
			{
				if (is_clock_register(i)) m_register[i] = m_userbuffer[i];
			}

			m_writing = false;
		}
	}
}

bool bq4847_device::is_clock_register(int regnum)
{
	return (regnum == reg_seconds || regnum == reg_minutes || regnum == reg_hours ||
		regnum == reg_date || regnum == reg_days || regnum == reg_month
		|| regnum == reg_year);
}

void bq4847_device::set_periodic_timer()
{
	uint8_t rs = m_register[reg_rates] & 0x0f;
	attotime period = rs ? clocks_to_attotime(1 << (rs - 1)) : attotime::never;

	if (m_periodic_timer)
		m_periodic_timer->adjust(period, 0, period);
}

void bq4847_device::set_watchdog_timer(int rst_state)
{
	if (m_rst_state == rst_state)
	{
		int wd = (m_register[reg_rates] & 0x70) >> 4;
		u32 t = (wd == 7) ? 16384 : (wd == 0) ? 8192 : 64 << wd;
		if (m_rst_state) t *= 6;
		attotime timeout = m_wdi_state >= 0 ? clocks_to_attotime(t) : attotime::never;

		if (m_watchdog_timer)
			m_watchdog_timer->adjust(timeout);
	}
}

void bq4847_device::set_wdo(int state)
{
	if (m_wdo_state != state)
	{
		m_wdo_state = state;
		m_wdo_handler(m_wdo_state);
	}
}

WRITE_LINE_MEMBER(bq4847_device::write_wdi)
{
	if (m_wdi_state != state)
	{
		m_wdi_state = state;

		set_wdo(1);
		set_watchdog_timer();
	}
}

TIMER_CALLBACK_MEMBER(bq4847_device::periodic_callback)
{
	m_userbuffer[reg_flags] |= FLAG_PF;
	update_int();
}

TIMER_CALLBACK_MEMBER(bq4847_device::watchdog_callback)
{
	m_rst_state = !m_rst_state;
	set_watchdog_timer(m_rst_state); // force timer update during reset

	m_rst_handler(m_rst_state);

	if (!m_rst_state)
		set_wdo(0);

	LOGMASKED(LOG_WATCHDOG, "wdo %s rst %s\n", !m_wdo_state ? "asserted" : "cleared", !m_rst_state ? "asserted" : "cleared");
}

void bq4847_device::update_int()
{
	// TODO: check what happens if reg_interrupts is changed after the flag is set.
	int int_state = !(m_register[reg_interrupts] & m_userbuffer[reg_flags] & (FLAG_AF | FLAG_PF | FLAG_PWRF));
	if (m_int_state != int_state)
	{
		m_int_state = int_state;
		m_int_handler(m_int_state);
	}
}

// device_t

void bq4847_device::device_start()
{
	m_update_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(bq4847_device::update_callback), this));
	m_periodic_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(bq4847_device::periodic_callback), this));
	m_watchdog_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(bq4847_device::watchdog_callback), this));

	m_int_handler.resolve_safe();
	m_wdo_handler.resolve_safe();
	m_rst_handler.resolve_safe();

	m_wdo_handler(m_wdo_state);
	m_int_handler(m_int_state);
	m_rst_handler(m_rst_state);

	save_pointer(NAME(m_userbuffer), 16);
	save_pointer(NAME(m_register), 16);
	save_item(NAME(m_wdo_state));
	save_item(NAME(m_int_state));
	save_item(NAME(m_rst_state));
	save_item(NAME(m_wdi_state));
	save_item(NAME(m_writing));
}

void bq4847_device::device_reset()
{
	device_clock_changed();
}

void bq4847_device::device_clock_changed()
{
	m_update_timer->adjust(clocks_to_attotime(32768), 0, clocks_to_attotime(32768));
	set_watchdog_timer();
	set_periodic_timer();
}

// device_nvram_interface

void bq4847_device::nvram_default()
{
	if (m_region.found())
	{
		if (m_region->bytes() != std::size(m_register))
			fatalerror("%s incorrect region size", tag());

		std::copy_n(m_region->base(), std::size(m_register), m_register);
	}
	else
	{
		std::fill_n(m_register, std::size(m_register), 0);

		m_register[reg_control] = CONTROL_STOP | CONTROL_24;
	}
}

bool bq4847_device::nvram_read(util::read_stream& file)
{
	size_t actual;
	return !file.read(m_register, std::size(m_register), actual) && actual == std::size(m_register);
}

bool bq4847_device::nvram_write(util::write_stream& file)
{
	size_t actual;
	return !file.write(m_register, std::size(m_register), actual) && actual == std::size(m_register);
}
