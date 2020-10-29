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
	FLAG_AIE = 0x08,
	FLAG_PIE = 0x04,
	FLAG_PWRIE = 0x02,
	FLAG_ABE = 0x01,
	FLAG_AF = 0x08,
	FLAG_PF = 0x04,
	FLAG_PWRF = 0x02,
	FLAG_BVF = 0x01,
	FLAG_UTI = 0x08,
	FLAG_STOP = 0x04,
	FLAG_24 = 0x02,
	FLAG_DSE = 0x01
};

//-------------------------------------------------
//  Constructors for basetype
//-------------------------------------------------

bq4847_device::bq4847_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BQ4847, tag, owner, clock),
	  device_nvram_interface(mconfig, *this),
	  device_rtc_interface(mconfig, *this),
	  m_interrupt_cb(*this),
	  m_wdout_cb(*this),
	  m_watchdog_active(false),
	  m_writing(false)
{
}

/*
    Inherited from device_rtc_interface. The date and time is given as integer
    and must be converted to BCD.
*/
void bq4847_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_intreg[reg_hours] = convert_to_bcd(hour);
	m_intreg[reg_minutes] = convert_to_bcd(minute);
	m_intreg[reg_seconds] = convert_to_bcd(second);
	m_intreg[reg_year] = convert_to_bcd(year);
	m_intreg[reg_month] = convert_to_bcd(month);
	m_intreg[reg_date] = convert_to_bcd(day);
	m_intreg[reg_days] = convert_to_bcd(day_of_week);
	// What about the DSE flag?
}

bool bq4847_device::increment_bcd(uint8_t& bcdnumber, uint8_t limit, uint8_t min)
{
/*  if (!valid_bcd(bcdnumber, min, limit))
    {
        bcdnumber = min;
        return false;
    }
*/
	if (bcdnumber>=limit)
	{
		bcdnumber = min;
		return true;
	}
	else
	{
		uint8_t dig0 = bcdnumber & 0x0f;
		uint8_t dig1 = bcdnumber & 0xf0;

		if (dig0==9)
		{
			bcdnumber = dig1 + 0x10;
		}
		else bcdnumber++;
	}
	return false;
}

// TODO: Remove; the real clock cannot verify BCD numbers.
bool bq4847_device::valid_bcd(uint8_t value, uint8_t min, uint8_t max)
{
	bool valid = ((value>=min) && (value<=max) && ((value&0x0f)<=9));
	if (!valid) LOGMASKED(LOG_WARN, "Invalid BCD number %02x\n", value);
	return valid;
}

// ----------------------------------------------------

/*
    Update cycle, called every second
    The BQ RTCs use BCD representation

    TODO: We may not be able to use the parent class advance methods, since we
    have to work with BCD (even with invalid values). Check this.
*/
TIMER_CALLBACK_MEMBER(bq4847_device::rtc_clock_cb)
{
	// Just for debugging
	static const char* dow[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

	bool carry = true;
	bool newsec = false;

	if (carry)
	{
		carry = increment_bcd(m_intreg[reg_seconds], 0x59, 0);
		newsec = true;
	}

	if (carry)
		carry = increment_bcd(m_intreg[reg_minutes], 0x59, 0);

	if (carry)
	{
		carry = advance_hours_bcd();
	}

	if (carry)
	{
		advance_days_bcd();
	}

	LOGMASKED(LOG_CLOCK, "%s 20%02x-%02x-%02x %02x:%02x:%02x\n",
		dow[m_intreg[reg_days]-1], m_intreg[reg_year], m_intreg[reg_month], m_intreg[reg_date],
		m_intreg[reg_hours], m_intreg[reg_minutes], m_intreg[reg_seconds]);

	// Copy into memory registers if the UTI bit is reset
	if (newsec)
	{
		if (!is_set(reg_control, FLAG_UTI))
		{
			// Copy values from internal registers to accessible registers
			transfer_to_access();
		}

		if (check_match(reg_date, reg_alarmdate) &&
			check_match(reg_hours, reg_alarmhours) &&
			check_match(reg_minutes, reg_alarmminutes) &&
			check_match(reg_seconds, reg_alarmseconds))
		{
			set_register(reg_flags, FLAG_AF, true);
			m_interrupt_cb(intrq_r());
		}
	}
}

bool bq4847_device::advance_hours_bcd()
{
	bool carry = false;
	// Handle DST
	if (is_set(reg_control, FLAG_DSE)
		&& (m_intreg[reg_month]==4) && (m_intreg[reg_days]==0) && (m_intreg[reg_date] < 8)  // first Sunday in April
		&& (m_intreg[reg_hours]==0x01))
		m_intreg[reg_hours] = 0x03;
	else
	{
		// Increment hour unless the DSE bit is set and we are at 1:59 on the last Sunday in October
		if (!is_set(reg_control, FLAG_DSE)
			|| (m_intreg[reg_month]!=10) || (m_intreg[reg_days]!=0) || (m_intreg[reg_date] <= 23)  // last Sunday in October
			|| (m_intreg[reg_hours]!=0x01))
		{
			if (is_set(reg_control, FLAG_24))
			{
				// 24h:  0->1->...->23->0(+1)
				increment_bcd(m_intreg[reg_hours], 0xff, 0);
				if (m_intreg[reg_hours] == 0x24)
				{
					m_intreg[reg_hours] = 0;
					carry = true;
				}
			}

			else
			{
				// 12h:  12->1->2->...->11->12'->1'->...->11'->12(+1)
				increment_bcd(m_intreg[reg_hours], 0xff, 0);
				switch (m_intreg[reg_hours])
				{
				case 0x12:
					m_intreg[reg_hours]=0x92;  // 11:59 am -> 12:00 pm
					break;
				case 0x93:
					m_intreg[reg_hours]=0x81;  // 12:59 pm -> 01:00 pm
					break;
				case 0x92:
					m_intreg[reg_hours]=0x12;  // 11:59 pm -> 12:00 am
					carry = true;
					break;
				case 0x13:
					m_intreg[reg_hours]=0x01;  // 12:59 am -> 01:00 am
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

	uint8_t month = bcd_to_integer(m_intreg[reg_month]);
	if (month > 12) month = 12;

	// if (!valid_bcd(month, 0x01, 0x12)) month = 1;
	uint8_t days = days_in_month_table[month-1];

	// Leap years are indeed handled (but the year is only 2-digit)
	if ((month==2) && ((m_intreg[reg_year]%4)==0))
		days = 0x29;

	increment_bcd(m_intreg[reg_days], 7, 1);  // Increment the day-of-week (without carry)
	carry = increment_bcd(m_intreg[reg_date], days, 1);

	if (carry)
	{
		increment_bcd(m_intreg[reg_month], 0xff, 1);
		if (m_intreg[reg_month] == 0x13)
		{
			m_intreg[reg_month] = 0x01;
			increment_bcd(m_intreg[reg_year], 0xff, 0);
		}
	}
}

bool bq4847_device::check_match(int now, int alarm)
{
	// The ignore feature is active once the alarm has set in
	// Will lead to a periodic alarm
	bool ignore = (is_set(m_reg[alarm], 0x80) && is_set(m_reg[alarm], 0x40)) && is_set(reg_flags, FLAG_AF);
	return ignore || (m_intreg[now] == (m_reg[alarm] & 0x3f));
}

// =========================================================

/*
    Read from registers
*/
uint8_t bq4847_device::read(offs_t address)
{
	int regnum = address & 0x0f;
	uint8_t value = m_reg[regnum];

	if (regnum == reg_flags)
	{
		set_register(reg_flags, 0xff, false);
		m_interrupt_cb(intrq_r());
	}
	else
		if (regnum == reg_unused) value = 0;  // Reg 15 is locked to 0 in BQ4847

	LOGMASKED(LOG_REG, "Reg %d -> %02x\n", regnum, value);

	return value;
}

/*
    Write to the registers
*/
void bq4847_device::write(offs_t address, uint8_t data)
{
	int regnum = address & 0x0f;

	LOGMASKED(LOG_REG, "Reg %d <- %02x\n", regnum, data);

	if (regnum == reg_flags)
	{
		LOGMASKED(LOG_WARN, "Ignoring write attempt to flag bit register (%02x)\n", data);
		return;
	}

	bool uti_set = is_set(reg_control, FLAG_UTI); // Get it before we change the flag

	m_reg[regnum] = data;

	if (regnum == reg_rates)
	{
		set_watchdog_timer(true);
		set_periodic_timer();
	}
	else
	{
		if (regnum == reg_control)
		{
			LOGMASKED(LOG_TRANSFER, "Update transfer %s\n", ((data & FLAG_UTI)!=0)? "inhibit" : "enable");

			// After we have written to the registers, transfer to the internal regs
			if (uti_set && ((data & FLAG_UTI)==0) && m_writing)
			{
				LOGMASKED(LOG_TRANSFER, "Transfer to internal regs\n");
				for (int i=reg_seconds; i < reg_unused; i++)
				{
					if (is_internal_register(i)) m_intreg[i] = m_reg[i];
				}
				// The real device does not auto-convert hours according to AM/PM
			}

			// We ignore the STOP* flag, since it only covers behaviour on power-off
			// We ignore the 24h/12h flag here; it requires reloading the registers anyway
			// The DSE flag will have effect on update
		}
		else
		{
			m_writing = true;
			// If inhibit is not set, any write to the time/date registers
			// is immediately set
			if (!uti_set && is_internal_register(regnum))
			{
				m_intreg[regnum] = m_reg[regnum];
			}
		}
	}
}

bool bq4847_device::is_internal_register(int regnum)
{
	return (regnum == reg_seconds || regnum == reg_minutes || regnum == reg_hours ||
				regnum == reg_date || regnum == reg_days || regnum == reg_month
				|| regnum == reg_year);
}

void bq4847_device::set_register(int number, uint8_t bits, bool set)
{
	if (set)
		m_reg[number] |= bits;
	else
		m_reg[number] &= ~bits;
}

bool bq4847_device::is_set(int number, uint8_t flag)
{
	return (m_reg[number] & flag)!=0;
}

void bq4847_device::transfer_to_access()
{
	LOGMASKED(LOG_TRANSFER, "Transfer to external regs\n");
	for (int i=reg_seconds; i < reg_unused; i++)
	{
		if (is_internal_register(i)) m_reg[i] = m_intreg[i];
	}
	// Clear the flag
	m_writing = false;
}

void bq4847_device::set_periodic_timer()
{
	uint8_t rateval = m_reg[reg_rates] & 0x0f;
	int rate = 1<<(16-rateval);

	if (rateval == 0)
		m_periodic_timer->reset();
	else
		m_periodic_timer->adjust(attotime::from_hz(rate), 0, attotime::from_hz(rate));
}

void bq4847_device::set_watchdog_timer(bool on)
{
	int val = (m_reg[reg_rates] & 0x70)>>4;

	// val = 0 -> 1.5 sec
	// val = 1 -> 3/128 sec
	// val = 2 -> 3/64 sec
	// ...
	// val = 6 -> 3/4 sec
	// val = 7 -> 3 sec

	s64 time = 250000000L;  // 250 ms
	if (val > 0)
	{
		time <<= 1;
		if (val < 7)
			time = time / (4<<(6-val));
	}

	if (on) time *= 6;  // delay to on is 6 times the delay to off

	if (m_watchdog_active)
		m_watchdog_timer->adjust(attotime::from_nsec(time)); // single shot
}

void bq4847_device::set_watchdog_active(bool active)
{
	m_watchdog_active = active;
}

void bq4847_device::retrigger_watchdog()
{
	m_wdout_cb(CLEAR_LINE);
	m_watchdog_asserted = false;
	set_watchdog_timer(true);
}

/*
    Periodic cycle (called at defined intervals)
*/
TIMER_CALLBACK_MEMBER(bq4847_device::rtc_periodic_cb)
{
	set_register(reg_flags, FLAG_PF, true);
	if (intrq_r())
		m_interrupt_cb(ASSERT_LINE);
}

/*
    Watchdog callback (BQ4847)
*/
TIMER_CALLBACK_MEMBER(bq4847_device::rtc_watchdog_cb)
{
	if (m_watchdog_active)
	{
		m_wdout_cb(m_watchdog_asserted? CLEAR_LINE : ASSERT_LINE);
		set_watchdog_timer(!m_watchdog_asserted);
		m_watchdog_asserted = !m_watchdog_asserted;
		LOGMASKED(LOG_WATCHDOG, "Watchdog %s\n", m_watchdog_asserted? "asserted" : "cleared");
	}
}

READ_LINE_MEMBER(bq4847_device::intrq_r)
{
	bool alarm = is_set(reg_interrupts, FLAG_AIE) && is_set(reg_flags, FLAG_AF);
	bool period = is_set(reg_interrupts, FLAG_PIE) && is_set(reg_flags, FLAG_PF);

	// We ignore interrupts from power fail or battery low
	return (alarm || period)? ASSERT_LINE : CLEAR_LINE;
}

void bq4847_device::connect_osc(bool conn)
{
	if (conn)
	{
		// The internal update cycle is 1 sec
		m_clock_timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));
		set_periodic_timer();
	}
	else
	{
		// Turn off completely
		m_clock_timer->reset();
		m_watchdog_timer->reset();
		m_periodic_timer->reset();
	}
}

void bq4847_device::device_start()
{
	m_clock_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(bq4847_device::rtc_clock_cb), this));

	// Periodic timer
	m_periodic_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(bq4847_device::rtc_periodic_cb), this));

	// Watchdog timer
	m_watchdog_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(bq4847_device::rtc_watchdog_cb), this));

	// Interrupt line
	m_interrupt_cb.resolve_safe();

	// Watchdog output
	m_wdout_cb.resolve_safe();

	// Interrupt enables are cleared on powerup
	set_register(reg_interrupts, 0xff, false);

	// State save
	save_pointer(NAME(m_reg), 16);
	save_pointer(NAME(m_intreg), 16);

	// Start clock
	connect_osc(true);
}

// ----------------------------------------------------

void bq4847_device::nvram_default()
{
	std::fill_n(m_reg, 16, 0);
}

void bq4847_device::nvram_read(emu_file &file)
{
	file.read(m_reg, 16);
	transfer_to_access();

	// Clear the saved flags
	set_register(reg_flags, 0xff, false);

	// Interrupts must be re-enabled on power-up
	set_register(reg_interrupts, 0xff, false);
}

void bq4847_device::nvram_write(emu_file &file)
{
	transfer_to_access();
	file.write(m_reg, 16);
}
