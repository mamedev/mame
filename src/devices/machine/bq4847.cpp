// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*
    Texas Instruments/Benchmarq BQ4847 Real-time clock

    Although featuring a similar interface, this chip is sufficiently
    different from the BQ4842/BQ4852 that a separate implementation
    makes sense.

    Supports 24h/12h and Daylight saving

    No internal memory, only clock registers

    Michael Zapf, April 2020
*/
#include "emu.h"
#include "bq4847.h"

#define LOG_WARN         (1U<<1)    // Warnings
#define LOG_CLOCK        (1U<<2)    // Clock operation
#define LOG_REGW         (1U<<3)    // Register write
#define LOG_WATCHDOG     (1U<<4)    // Watchdog

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
	reg_hours,              // 0x00 - 0x23 (24h) or 0x81 - 0x92 (12h)
	reg_alarmhours,         // 0xc0 to ignore
	reg_date,               // 0x01 - 0x31
	reg_alarmdate,          // 0xc0 to ignore
	reg_days,               // 0x01 (sun) - 0x07 (sat)
	reg_month,              // 0x01 - 0x12
	reg_year,               // 0x00 - 0x99
	reg_rates,              // 0 [--WD--] [-----RS---------]
	reg_interrupts,         // 0  0  0  0 AIE  PIE PWRIE ABE    0x00 on powerup
	reg_flags,              // 0  0  0  0  AF   PF  PWRF BVF    0x00 after reading
	reg_control,            // 0  0  0  0 UTI STOP 24/12 DSE
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
	  m_interrupt_cb(*this),
	  m_wdout_cb(*this),
	  m_watchdog_active(false),
	  m_writing(false)
{
}

bool bq4847_device::increment_bcd(uint8_t& bcdnumber, uint8_t limit, uint8_t min)
{
	if (!valid_bcd(bcdnumber, min, limit))
	{
		bcdnumber = min;
		return false;
	}

	if (bcdnumber==limit)
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

bool bq4847_device::valid_bcd(uint8_t value, uint8_t min, uint8_t max)
{
	bool valid = ((value>=min) && (value<=max) && ((value&0x0f)<=9));
	if (!valid) LOGMASKED(LOG_WARN, "Invalid BCD number %02x\n", value);
	return valid;
}

uint8_t bq4847_device::to_bcd(uint8_t value)
{
	return (((value / 10) << 4) & 0xf0) | (value % 10);
}

uint8_t bq4847_device::from_bcd(uint8_t value)
{
	return ((value & 0xf0)>>4)*10 + (value & 0x0f);
}

// ----------------------------------------------------

/*
    Update cycle, called every second
    The BQ RTCs use BCD representation
*/
TIMER_CALLBACK_MEMBER(bq4847_device::rtc_clock_cb)
{
	// BCD-encoded numbers
	static const int days_in_month_table[12] =
	{
		0x31,0x28,0x31, 0x30,0x31,0x30,
		0x31, 0x31, 0x30, 0x31, 0x30, 0x31
	};

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
		// Handle DST
		if (is_set(reg_control, FLAG_DSE)
			 && (m_reg[reg_month]==4) && (m_reg[reg_days]==0) && (m_reg[reg_date] < 8)  // first Sunday in April
			&& (m_reg[reg_hours]==0x01))
			m_reg[reg_hours] = 0x03;
		else
		{
			if (!is_set(reg_control, FLAG_DSE)
				|| (m_reg[reg_month]!=10) || (m_reg[reg_days]!=0) || (m_reg[reg_date] <= 23)  // last Sunday in October
				|| (m_reg[reg_hours]!=0x01))
				carry = increment_bcd(m_intreg[reg_hours], 0x23, 0);
		}
	}

	if (carry)
	{
		uint8_t month = m_intreg[reg_month];
		if (!valid_bcd(month, 0x01, 0x12)) month = 1;
		uint8_t days = days_in_month_table[month-1];

		// Are leap years considered?
		if ((month==2)
			&& ((m_intreg[reg_year]%4)==0)
			&& ((m_intreg[reg_year]%100)!=0
			   || (m_intreg[reg_year]%400)==0))
			days = 0x29;

		increment_bcd(m_intreg[reg_days], 7, 1);
		carry = increment_bcd(m_intreg[reg_date], days, 1);
	}
	if (carry)
		carry = increment_bcd(m_intreg[reg_month], 0x12, 1);
	if (carry)
		carry = increment_bcd(m_intreg[reg_year], 0x99, 0);

	LOGMASKED(LOG_CLOCK, "%s 20%02x-%02x-%02x %02x:%02x:%02x\n",
		dow[m_intreg[reg_days]-1], m_intreg[reg_year], m_intreg[reg_month], m_intreg[reg_date],
		m_intreg[reg_hours], m_intreg[reg_minutes], m_intreg[reg_seconds]);

	// Copy into memory registers if the read bit is reset
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

	return value;
}

/*
    Write to the registers
*/
void bq4847_device::write(offs_t address, uint8_t data)
{
	int regnum = address & 0x0f;

	if (regnum == reg_flags)
	{
		LOGMASKED(LOG_WARN, "Ignoring write attempt to flag bit register (%02x)\n", data);
		return;
	}

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
			// After we have written to the registers, transfer to the internal regs
			if (is_set(reg_control, FLAG_UTI) && ((data & FLAG_UTI)==0) && m_writing)
				transfer_to_int();

			// We ignore the STOP* flag, since it only covers behaviour on power-off
			// We ignore the 24h/12h flag here; it requires reloading the registers anyway
			// The DSE flag will have effect on update
		}
		else
			m_writing = true;
	}
}

void bq4847_device::set_register(int number, uint8_t bits, bool set)
{
	if (set)
		m_reg[number] |= bits;
	else
		m_reg[number] &= ~bits;
}

uint8_t bq4847_device::ampmto24(uint8_t ampm)
{
	uint8_t f24 = from_bcd(ampm);

	if (f24==12) f24 = 0;
	else
	{
		if (ampm & 0x80)
		{
			if (f24 == 92)
				f24 = 12;
			else
				f24 = f24 - 68;
		}
	}
	return to_bcd(f24);
}

uint8_t bq4847_device::ampmfrom24(uint8_t f24)
{
	uint8_t ampm = from_bcd(f24);
	if (ampm==0)
		ampm = 12;
	else
	{
		if (ampm == 12)
			ampm = 92;
		else
		{
			if (ampm > 12)
				ampm = ampm + 68;
		}
	}
	return to_bcd(ampm);
}

bool bq4847_device::is_set(int number, uint8_t flag)
{
	return (m_reg[number] & flag)!=0;
}

void bq4847_device::transfer_to_int()
{
	m_intreg[reg_year] = m_reg[reg_year];
	m_intreg[reg_month] = m_reg[reg_month];
	m_intreg[reg_date] = m_reg[reg_date];
	m_intreg[reg_days] = m_reg[reg_days];
	m_intreg[reg_minutes] = m_reg[reg_minutes];
	m_intreg[reg_seconds] = m_reg[reg_seconds];

	// Check: What is the real device's behavior on inconsistent time formats?
	if (is_set(reg_control, FLAG_24))
		m_intreg[reg_hours] = m_reg[reg_hours];
	else
		m_intreg[reg_hours] = ampmto24(m_reg[reg_hours]);
}

void bq4847_device::transfer_to_access()
{
	m_reg[reg_year] = m_intreg[reg_year];
	m_reg[reg_month] = m_intreg[reg_month];
	m_reg[reg_date] = m_intreg[reg_date];
	m_reg[reg_days] = m_intreg[reg_days];
	m_reg[reg_minutes] = m_intreg[reg_minutes];
	m_reg[reg_seconds] = m_intreg[reg_seconds];

	// Convert to AM/PM if selected
	if (is_set(reg_control, FLAG_24))
		m_reg[reg_hours] = m_intreg[reg_hours];
	else
		m_reg[reg_hours] = ampmfrom24(m_intreg[reg_hours]);

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

void bq4847_device::get_system_time()
{
	// Set time from system time
	// We always use 24h internally
	system_time systime;
	machine().current_datetime(systime);
	m_intreg[reg_hours] = to_bcd(systime.local_time.hour);
	m_intreg[reg_minutes] = to_bcd(systime.local_time.minute);
	m_intreg[reg_seconds] = to_bcd(systime.local_time.second);
	m_intreg[reg_year] = to_bcd(systime.local_time.year%100);
	m_intreg[reg_month] = to_bcd(systime.local_time.month+1);
	m_intreg[reg_date] = to_bcd(systime.local_time.mday);
	m_intreg[reg_days] = to_bcd(systime.local_time.weekday+1);

	set_register(reg_control, FLAG_DSE, systime.local_time.is_dst);
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
	get_system_time();
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
	get_system_time();
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
