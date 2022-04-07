// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*
    Texas Instruments/Benchmarq BQ4842/52 Real-time clock

    Michael Zapf, April 2020
*/
#include "emu.h"
#include "bq48x2.h"

#define LOG_WARN         (1U<<1)    // Warnings
#define LOG_CLOCK        (1U<<2)    // Clock operation
#define LOG_REGW         (1U<<3)    // Register write
#define LOG_WATCHDOG     (1U<<4)    // Watchdog
#define LOG_SRAM         (1U<<5)    // SRAM

#define VERBOSE ( LOG_GENERAL | LOG_WARN )
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(BQ4842, bq4842_device, "bq4842", "Benchmarq BQ4842 RTC")
DEFINE_DEVICE_TYPE(BQ4852, bq4852_device, "bq4852", "Benchmarq BQ4852 RTC")

enum
{
	reg_year = 0,
	reg_month,
	reg_date,
	reg_days,
	reg_hours,
	reg_minutes,
	reg_seconds,
	reg_control,
	reg_watchdog,
	reg_interrupts,
	reg_alarmdate,
	reg_alarmhours,
	reg_alarmminutes,
	reg_alarmseconds,
	reg_100ths,
	reg_flags
};

enum
{
	FLAG_FTE = 0x40,
	FLAG_OSC = 0x80,
	FLAG_W = 0x80,
	FLAG_R = 0x40,
	FLAG_WDS = 0x80,
	FLAG_AIE = 0x80,
	FLAG_PIE = 0x10,
	FLAG_AF = 0x40,
	FLAG_WDF = 0x80,
	FLAG_PF = 0x08
};

//-------------------------------------------------
//  Constructors for basetype
//-------------------------------------------------

bq48x2_device::bq48x2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, int memsize)
	: device_t(mconfig, type, tag, owner, 0),
	  device_nvram_interface(mconfig, *this),
	  device_rtc_interface(mconfig, *this),
	  m_interrupt_cb(*this),
	  m_resetout_cb(*this),
	  m_memsize(memsize)
{
}

//-------------------------------------------------
//  Constructors for subtypes
//-------------------------------------------------

// 128 KiB memory (including clock registers)
bq4842_device::bq4842_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bq48x2_device(mconfig, BQ4842, tag, owner, 128*1024)
{
}

// 512 KiB memory (including clock registers)
bq4852_device::bq4852_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bq48x2_device(mconfig, BQ4852, tag, owner, 512*1024)
{
}

/*
    Inherited from device_rtc_interface. The date and time is given as integer
    and must be converted to BCD.
*/
void bq48x2_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_intreg[reg_hours] = convert_to_bcd(hour);
	m_intreg[reg_minutes] = convert_to_bcd(minute);
	m_intreg[reg_seconds] = convert_to_bcd(second);
	m_intreg[reg_year] = convert_to_bcd(year);
	m_intreg[reg_month] = convert_to_bcd(month);
	m_intreg[reg_date] = convert_to_bcd(day);
	m_intreg[reg_days] = convert_to_bcd(day_of_week);
}

bool bq48x2_device::increment_bcd(uint8_t& bcdnumber, uint8_t limit, uint8_t min)
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

// TODO: Remove; the real clock cannot verify BCD numbers.
bool bq48x2_device::valid_bcd(uint8_t value, uint8_t min, uint8_t max)
{
	bool valid = ((value>=min) && (value<=max) && ((value&0x0f)<=9));
	if (!valid) LOGMASKED(LOG_WARN, "Invalid BCD number %02x\n", value);
	return valid;
}

// ----------------------------------------------------

/*
    Update cycle, called every second
    The BQ RTCs use BCD representation
*/
TIMER_CALLBACK_MEMBER(bq48x2_device::rtc_clock_cb)
{
	// Just for debugging
	static const char* dow[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

	bool carry = true;
	bool newsec = false;

	// Test mode (FTW) or oscillator stop (OSC)
	if (get_register(reg_days, FLAG_FTE) || get_register(reg_seconds, FLAG_OSC))
		return;

	// When the timer ticks, the 100ths are 0.
	// TODO: Verify this with a real chip
	m_intreg[reg_100ths] = 0;

	if (carry)
	{
		carry = increment_bcd(m_intreg[reg_seconds], 0x59, 0);
		newsec = true;
	}
	if (carry)
		carry = increment_bcd(m_intreg[reg_minutes], 0x59, 0);

	if (carry)
	{
		increment_bcd(m_intreg[reg_hours], 0xff, 0);
		if (m_intreg[reg_hours] == 0x24)
		{
			m_intreg[reg_hours] = 0;
			carry = true;
		}
	}
	if (carry)
	{
		advance_days_bcd();
	}

	LOGMASKED(LOG_CLOCK, "%s 20%02x-%02x-%02x %02x:%02x:%02x\n",
		dow[m_intreg[reg_days]-1], m_intreg[reg_year], m_intreg[reg_month], m_intreg[reg_date],
		m_intreg[reg_hours], m_intreg[reg_minutes], m_intreg[reg_seconds]);

	// Copy into memory registers if the read bit is reset
	if (newsec)
	{
		if (!is_set(reg_control, FLAG_R | FLAG_W))
		{
			// Copy values from internal registers to memory space
			transfer_to_access();
		}

		if (check_match(reg_date, reg_alarmdate, 0x3f) &&
			check_match(reg_hours, reg_alarmhours, 0x3f) &&
			check_match(reg_minutes, reg_alarmminutes, 0x7f) &&
			check_match(reg_seconds, reg_alarmseconds, 0x7f))
		{
			set_register(reg_flags, FLAG_AF, true);
			m_interrupt_cb(intrq_r());
		}
	}
}
void bq48x2_device::advance_days_bcd()
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

uint8_t bq48x2_device::get_register(int number, uint8_t mask)
{
	return m_sram[m_memsize-1-number] & mask;
}

bool bq48x2_device::is_set(int number, uint8_t flag)
{
	return get_register(number, flag)!=0;
}

void bq48x2_device::set_register(int number, uint8_t bits, bool set)
{
	int addr = m_memsize-1-number;

	if (set)
		m_sram[addr] |= bits;
	else
		m_sram[addr] &= ~bits;
}

void bq48x2_device::set_register(int number, uint8_t value)
{
	m_sram[m_memsize-1-number] = value;
}

// The 0 bits in these masks are the "unused bits" according to the specification;
// they are left unchanged
static const uint8_t regmask[] = { 0xff, 0x1f, 0x3f, 0x07, 0x3f, 0x7f, 0x7f, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xf8 };

void bq48x2_device::transfer_to_int()
{
	uint8_t hds = m_intreg[reg_100ths];

	for (int i=0; i < 16; i++)
		m_intreg[i] = get_register(i, regmask[i]);

	// If we set the 100ths not to be 0, the next second will occur earlier
	// TODO: Check this with the real chip
	if (hds != m_intreg[reg_100ths])
		m_clock_timer->adjust(attotime::from_msec(get_delay()), 0, attotime::from_seconds(1));
}

void bq48x2_device::transfer_to_access()
{
	for (int i=0; i < 16; i++)
		set_register(i, get_register(i, ~regmask[i]) |  (m_intreg[i] & regmask[i]));
}

bool bq48x2_device::check_match(int now, int alarm, uint8_t mask)
{
	// The ignore feature is active once the alarm has set in
	// Will lead to a periodic alarm
	bool ignore = (is_set(alarm, 0x80) && is_set(reg_flags, FLAG_AF));
	return ignore || ((m_intreg[now] & mask) == get_register(alarm, mask));
}

// =========================================================

/*
    Read from SRAM or registers
*/
uint8_t bq48x2_device::read(offs_t address)
{
	address = address & (m_memsize-1);

	uint8_t value = m_sram[address];

	if ((m_memsize-1-address) == reg_flags)   // Read flag register
	{
		set_register(reg_flags, 0xf8, false); // reset all flags
		m_interrupt_cb(intrq_r());
	}
	return value;
}

/*
    Write to the SRAM or registers
*/
void bq48x2_device::write(offs_t address, uint8_t data)
{
	address = address % m_memsize;

	int regmask = (m_memsize - 1) & ~0x0f;

	// Registers
	if ((address & regmask) == regmask)
	{
		int regnum = 15 - (address & 0x0f);
		switch (regnum)
		{
		// No special effect
		case reg_year:
		case reg_month:
		case reg_date:
		case reg_hours:
		case reg_minutes:
		case reg_alarmdate:
		case reg_alarmhours:
		case reg_alarmminutes:
		case reg_alarmseconds:
		case reg_100ths:
			break;

		case reg_days:
			if (data & FLAG_FTE)
				// Test mode
				m_periodic_timer->adjust(attotime::from_hz(1024), 0, attotime::from_hz(1024));
			else
			{
				// reset to periodic timing
				set_periodic_timer();
			}
			break;
		case reg_seconds:
			// Start oscillator on falling edge
			if (is_set(reg_seconds, FLAG_OSC) && ((data & FLAG_OSC) == 0))
				connect_osc(true);
			else
			{
				// Turn off oscillator on raising edge
				if (!is_set(reg_seconds, FLAG_OSC) && ((data & FLAG_OSC) != 0))
					connect_osc(false);
			}
			break;
		case reg_control:
			// Transfer to internal registers when W set to 0
			if (is_set(reg_control, FLAG_W) && ((data & FLAG_W) == 0))
				transfer_to_int();
			// Calibration bits are ignored, we don't calibrate the
			// backing PC clock
			break;
		case reg_watchdog:
			set_register(regnum, data);
			set_watchdog_timer();
			break;

		case reg_interrupts:
			set_register(regnum, data);
			set_periodic_timer();
			return;
		case reg_flags:
			LOGMASKED(LOG_WARN, "Ignoring write attempt to flag bit register (%02x)\n", data);
			return;
		}
		set_register(regnum, data);
	}
	else
	{
		LOGMASKED(LOG_SRAM, "sram %05x <- %02x\n", address, data);
		m_sram[address] = data;
	}
}

void bq48x2_device::set_periodic_timer()
{
	uint8_t rateval = get_register(reg_interrupts, 0x0f);
	int rate = 0;

	switch (rateval)
	{
	case 0:
		m_periodic_timer->reset();
		break;
	case 1:
		m_periodic_timer->adjust(attotime::from_msec(10), 0, attotime::from_msec(10));
		break;
	case 2:
		m_periodic_timer->adjust(attotime::from_msec(100), 0, attotime::from_msec(100));
		break;
	default:
		rate = 1 << (16-rateval);
		m_periodic_timer->adjust(attotime::from_hz(rate), 0, attotime::from_hz(rate));
		break;
	}
}

void bq48x2_device::set_watchdog_timer()
{
	int multi = get_register(reg_watchdog, 0x7c)>>2;
	int reso = get_register(reg_watchdog, 0x03);

	// reso = 0  -> 1/16 s  (2^-4)  = 62500 us
	// reso = 1  -> 1/4 s   (2^-2)
	// reso = 2  -> 1 s     (2^0)
	// reso = 3  -> 4 s     (2^2)

	int time = (1<<(reso*2))*62500 * multi;
	m_watchdog_timer->adjust(attotime::from_usec(time)); // single shot
}

/*
    Periodic cycle (called at defined intervals)
*/
TIMER_CALLBACK_MEMBER(bq48x2_device::rtc_periodic_cb)
{
	// Test mode
	if (get_register(reg_days, FLAG_FTE))
	{
		// Create a 1:1 on-off signal on the seconds' last bit
		set_register(reg_seconds, get_register(reg_seconds, 0xff) ^ 0x01);
	}
	else
	{
		set_register(reg_flags, FLAG_PF, true);
		// The INT line is only released by reading the flag register
		if (intrq_r())
		{
			m_interrupt_cb(ASSERT_LINE);
		}
	}
}

/*
    Watchdog callback
*/
TIMER_CALLBACK_MEMBER(bq48x2_device::rtc_watchdog_cb)
{
	set_register(reg_flags, FLAG_WDF, true);
	if (is_set(reg_watchdog, FLAG_WDS))
	{
		LOGMASKED(LOG_WATCHDOG, "Watchdog alarm, reset pulse\n");
		m_resetout_cb(ASSERT_LINE);
		// During the reset pulse, the watchdog register is cleared
		set_register(reg_watchdog, 0);
		m_resetout_cb(CLEAR_LINE);
	}
	else
	{
		LOGMASKED(LOG_WATCHDOG, "Watchdog alarm, interrupt\n");
		m_interrupt_cb(intrq_r());
	}
}

/*
    Indicates that there is an interrupt condition. Also used to drive the
    outgoing line.
*/
READ_LINE_MEMBER(bq48x2_device::intrq_r)
{
	bool alarm = (is_set(reg_interrupts, FLAG_AIE) && is_set(reg_flags, FLAG_AF));
	bool period = (is_set(reg_interrupts, FLAG_PIE) && is_set(reg_flags, FLAG_PF));

	return (alarm || period)? ASSERT_LINE : CLEAR_LINE;
}

void bq48x2_device::connect_osc(bool conn)
{
	if (conn)
	{
		// The internal update cycle is 1 sec
		m_clock_timer->adjust(attotime::from_msec(get_delay()), 0, attotime::from_seconds(1));
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

int bq48x2_device::get_delay()
{
	int hds = ((m_intreg[reg_100ths] & 0xf0)>>16) * 10 + (m_intreg[reg_100ths] & 0x0f);
	return 1000 - hds*10;
}

void bq48x2_device::device_start()
{
	m_clock_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(bq48x2_device::rtc_clock_cb), this));

	// Periodic timer
	m_periodic_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(bq48x2_device::rtc_periodic_cb), this));

	// Watchdog timer
	m_watchdog_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(bq48x2_device::rtc_watchdog_cb), this));

	// Interrupt line
	m_interrupt_cb.resolve_safe();

	// Reset output
	m_resetout_cb.resolve_safe();

	m_sram = std::make_unique<u8 []>(m_memsize);

	// Interrupt enables are cleared on powerup
	set_register(reg_interrupts, 0xff, false);

	// State save
	save_pointer(NAME(m_sram), m_memsize);
	save_pointer(NAME(m_intreg), 8);

	// Start clock
	connect_osc(true);
}

// ----------------------------------------------------

void bq48x2_device::nvram_default()
{
	std::fill_n(m_sram.get(), m_memsize, 0);
}

bool bq48x2_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	if (file.read(m_sram.get(), m_memsize, actual) || actual != m_memsize)
		return false;

	transfer_to_access();  // Transfer the system time into the readable registers

	// Clear the saved flags
	set_register(reg_flags, 0xf8, true);

	return true;
}

bool bq48x2_device::nvram_write(util::write_stream &file)
{
	transfer_to_access();

	size_t actual;
	return !file.write(m_sram.get(), m_memsize, actual) && actual == m_memsize;
}
