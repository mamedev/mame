// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    Dallas DS1386/DS1386P RAMified Watchdog Timekeeper

    Note: Largely untested.

**********************************************************************/

#include "emu.h"
#include "ds1386.h"
#include "machine/timehelp.h"

#define DISABLE_OSC     (0x80)
#define DISABLE_SQW     (0x40)

#define COMMAND_TE      (0x80)
#define COMMAND_IPSW    (0x40)
#define COMMAND_IBH_LO  (0x20)
#define COMMAND_PU_LVL  (0x10)
#define COMMAND_WAM     (0x08)
#define COMMAND_TDM     (0x04)
#define COMMAND_WAF     (0x02)
#define COMMAND_TDF     (0x01)

#define HOURS_12_24     (0x40)
#define HOURS_AM_PM     (0x20)

DEFINE_DEVICE_TYPE(DS1286,     ds1286_device,     "ds1286",     "DS1286 Watchdog Timekeeper")
DEFINE_DEVICE_TYPE(DS1386_8K,  ds1386_8k_device,  "ds1386_8k",  "DS1386-8K RAMified Watchdog Timekeeper")
DEFINE_DEVICE_TYPE(DS1386_32K, ds1386_32k_device, "ds1386_32k", "DS1386-32K RAMified Watchdog Timekeeper")

ds1386_device::ds1386_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, size_t size)
	: device_t(mconfig, type, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, device_rtc_interface(mconfig, *this)
	, m_tod_alarm(0)
	, m_watchdog_alarm(0)
	, m_square(1)
	, m_inta_cb(*this)
	, m_intb_cb(*this)
	, m_sqw_cb(*this)
	, m_clock_timer(nullptr)
	, m_square_timer(nullptr)
	, m_watchdog_timer(nullptr)
	, m_inta_timer(nullptr)
	, m_intb_timer(nullptr)
	, m_default_data(*this, DEVICE_SELF)
	, m_hundredths(0)
	, m_seconds(0)
	, m_minutes(0)
	, m_minutes_alarm(0)
	, m_hours(0)
	, m_hours_alarm(0)
	, m_days(0)
	, m_days_alarm(0)
	, m_date(0)
	, m_months_enables(0)
	, m_years(0)
	, m_ram_size(size)
{
}

ds1286_device::ds1286_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ds1386_device(mconfig, DS1286, tag, owner, clock, 64)
{
}

ds1386_8k_device::ds1386_8k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ds1386_device(mconfig, DS1386_8K, tag, owner, clock, 8*1024)
{
}

ds1386_32k_device::ds1386_32k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ds1386_device(mconfig, DS1386_32K, tag, owner, clock, 32*1024)
{
}

void ds1386_device::safe_inta_cb(int state)
{
	if (!m_inta_cb.isnull())
		m_inta_cb(state);
}

void ds1386_device::safe_intb_cb(int state)
{
	if (!m_intb_cb.isnull())
		m_intb_cb(state);
}

void ds1386_device::safe_sqw_cb(int state)
{
	if (!m_sqw_cb.isnull())
		m_sqw_cb(state);
}

void ds1386_device::device_start()
{
	m_inta_cb.resolve();
	m_intb_cb.resolve();
	m_sqw_cb.resolve();

	m_tod_alarm = 0;
	m_watchdog_alarm = 0;
	m_square = 1;

	safe_inta_cb(0);
	safe_intb_cb(0);
	safe_sqw_cb(1);

	// allocate timers
	m_clock_timer = timer_alloc(CLOCK_TIMER);
	m_clock_timer->adjust(attotime::from_hz(100), 0, attotime::from_hz(100));
	m_square_timer = timer_alloc(SQUAREWAVE_TIMER);
	m_square_timer->adjust(attotime::never);
	m_watchdog_timer = timer_alloc(WATCHDOG_TIMER);
	m_watchdog_timer->adjust(attotime::never);
	m_inta_timer= timer_alloc(INTA_TIMER);
	m_inta_timer->adjust(attotime::never);
	m_intb_timer= timer_alloc(INTB_TIMER);
	m_intb_timer->adjust(attotime::never);

	// state saving
	save_item(NAME(m_tod_alarm));
	save_item(NAME(m_watchdog_alarm));
	save_item(NAME(m_square));

	m_ram = std::make_unique<uint8_t[]>(m_ram_size);
	save_pointer(NAME(m_ram), m_ram_size);
}


void ds1386_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_hundredths = 0;
	m_seconds = time_helper::make_bcd(second);
	m_minutes = time_helper::make_bcd(minute);
	if (m_hours & HOURS_12_24)
	{
		if (hour >= 12)
			m_hours = time_helper::make_bcd(hour == 12 ? 12 : hour - 12) | HOURS_12_24 | HOURS_AM_PM;
		else
			m_hours = time_helper::make_bcd(hour == 0 ? 12 : hour) | HOURS_12_24;
	}
	else
		m_hours = time_helper::make_bcd(hour);
	m_days = time_helper::make_bcd(day_of_week);
	m_date = time_helper::make_bcd(second);
	m_months_enables = (time_helper::make_bcd(month) & 0x1f) | (m_months_enables & 0xc0);
	m_years = time_helper::make_bcd(year);

	copy_registers_to_ram();
}

void ds1386_device::time_of_day_alarm()
{
	m_ram[REGISTER_COMMAND] |= COMMAND_TDF;
	m_tod_alarm = 1;

	if (m_ram[REGISTER_COMMAND] & COMMAND_IPSW)
	{
		safe_inta_cb(m_tod_alarm);
		if (m_ram[REGISTER_COMMAND] & COMMAND_PU_LVL)
		{
			m_inta_timer->adjust(attotime::from_msec(3));
		}
	}
	else
	{
		safe_intb_cb(m_tod_alarm);
		if (m_ram[REGISTER_COMMAND] & COMMAND_PU_LVL)
		{
			m_intb_timer->adjust(attotime::from_msec(3));
		}
	}
}

void ds1386_device::watchdog_alarm()
{
	m_ram[REGISTER_COMMAND] |= COMMAND_WAF;
	m_watchdog_alarm = 1;

	if (m_ram[REGISTER_COMMAND] & COMMAND_IPSW)
	{
		safe_intb_cb(m_watchdog_alarm);
		if (m_ram[REGISTER_COMMAND] & COMMAND_PU_LVL)
		{
			m_intb_timer->adjust(attotime::from_msec(3));
		}
	}
	else
	{
		safe_inta_cb(m_watchdog_alarm);
		if (m_ram[REGISTER_COMMAND] & COMMAND_PU_LVL)
		{
			m_inta_timer->adjust(attotime::from_msec(3));
		}
	}
}

void ds1386_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
		case CLOCK_TIMER:
			advance_hundredths();
			break;

		case SQUAREWAVE_TIMER:
			m_square = ((m_square == 0) ? 1 : 0);
			safe_sqw_cb(m_square);
			break;

		case WATCHDOG_TIMER:
			if ((m_ram[REGISTER_COMMAND] & COMMAND_WAF) == 0)
				watchdog_alarm();
			break;

		case INTA_TIMER:
			if (m_ram[REGISTER_COMMAND] & COMMAND_IPSW)
			{
				m_tod_alarm = 0;
			}
			else
			{
				m_watchdog_alarm = 0;
			}
			safe_inta_cb(0);
			break;

		case INTB_TIMER:
			if (m_ram[REGISTER_COMMAND] & COMMAND_IPSW)
			{
				m_watchdog_alarm = 0;
			}
			else
			{
				m_tod_alarm = 0;
			}
			safe_intb_cb(0);
			break;
	}
}

void ds1386_device::advance_hundredths()
{
	if ((m_ram[REGISTER_COMMAND] & COMMAND_TE) != 0)
	{
		copy_ram_to_registers();
	}

	int carry = time_helper::inc_bcd(&m_hundredths, 0xff, 0x00, 0x99);
	if (carry)
	{
		carry = time_helper::inc_bcd(&m_seconds, 0x7f, 0x00, 0x59);
	}
	if (carry)
	{
		carry = time_helper::inc_bcd(&m_minutes, 0x7f, 0x00, 0x59);
	}
	if (carry)
	{
		uint8_t value = 0;
		uint8_t min_value = 0;
		uint8_t max_value = 0;
		if (m_hours & HOURS_12_24)
		{
			value = m_hours & 0x1f;
			min_value = 0x01;
			max_value = 0x12;
		}
		else
		{
			value = m_hours & 0x3f;
			min_value = 0x00;
			max_value = 0x23;
		}
		carry = time_helper::inc_bcd(&value, 0x1f, min_value, max_value);

		m_hours &= ~0x1f;
		if (carry)
		{
			if (m_hours & HOURS_12_24)
			{
				if (m_hours & HOURS_AM_PM)
					carry = 1;
				else
					carry = 0;

				m_hours = m_hours ^ HOURS_AM_PM;
			}
			else
			{
				m_hours &= ~0x3f;
			}
		}
		else
		{
			if ((m_hours & HOURS_12_24) == 0)
				m_hours &= ~0x3f;
		}
		m_hours |= value;
	}

	if (carry)
	{
		uint8_t maxdays;
		static const uint8_t daysinmonth[] = { 0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31 };

		time_helper::inc_bcd(&m_days, 0x07, 0x01, 0x07);

		uint8_t month = time_helper::from_bcd(m_months_enables);
		uint8_t year = time_helper::from_bcd(m_years);

		if (month == 2 && (year % 4) == 0)
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

		carry = time_helper::inc_bcd(&m_date, 0x3f, 0x01, maxdays);
	}
	if (carry)
	{
		uint8_t enables = m_months_enables & 0xc0;
		carry = time_helper::inc_bcd(&m_months_enables, 0x1f, 0x01, 0x12);
		m_months_enables &= 0x3f;
		m_months_enables |= enables;
	}
	if (carry)
	{
		carry = time_helper::inc_bcd(&m_years, 0xff, 0x00, 0x99);
	}

	if ((m_ram[REGISTER_COMMAND] & COMMAND_TE) != 0)
	{
		copy_registers_to_ram();
	}

	check_tod_alarm();
}

void ds1386_device::copy_ram_to_registers()
{
	m_hundredths = m_ram[REGISTER_HUNDREDTHS];
	m_seconds = m_ram[REGISTER_SECONDS];
	m_minutes = m_ram[REGISTER_MINUTES];
	m_hours = m_ram[REGISTER_HOURS];
	m_days = m_ram[REGISTER_DAYS];
	m_date = m_ram[REGISTER_DATE];
	m_months_enables = m_ram[REGISTER_MONTHS];
	m_years = m_ram[REGISTER_YEARS];
}

void ds1386_device::copy_registers_to_ram()
{
	m_ram[REGISTER_HUNDREDTHS] = m_hundredths;
	m_ram[REGISTER_SECONDS] = m_seconds;
	m_ram[REGISTER_MINUTES] = m_minutes;
	m_ram[REGISTER_HOURS] = m_hours;
	m_ram[REGISTER_DAYS] = m_days;
	m_ram[REGISTER_DATE] = m_date;
	m_ram[REGISTER_MONTHS] = m_months_enables;
	m_ram[REGISTER_YEARS] = m_years;
}

void ds1386_device::check_tod_alarm()
{
	uint8_t mode = BIT(m_days_alarm, 7) | (BIT(m_hours_alarm, 5) << 1) | (BIT(m_minutes_alarm, 3) << 2);
	bool zeroes = (m_hundredths == 0 && m_seconds == 0);
	if (zeroes && (m_ram[REGISTER_COMMAND] & COMMAND_TDF) == 0)
	{
		bool minutes_match = (m_minutes & 0x7f) == (m_minutes_alarm & 0x7f);
		bool hours_match = (m_hours & 0x7f) == (m_hours_alarm & 0x7f);
		bool days_match = (m_days & 0x7) == (m_days_alarm & 0x07);
		bool alarm_match = false;
		switch (mode)
		{
			case ALARM_PER_MINUTE:
				alarm_match = true;
				break;
			case ALARM_MINUTES_MATCH:
				alarm_match = minutes_match;
				break;
			case ALARM_HOURS_MATCH:
				alarm_match = hours_match;
				break;
			case ALARM_DAYS_MATCH:
				alarm_match = days_match;
				break;
			default:
				break;
		}
		if (alarm_match)
		{
			time_of_day_alarm();
		}
	}
}

void ds1386_device::nvram_default()
{
	std::fill_n(&m_ram[0], m_ram_size, 0);
	m_ram[REGISTER_COMMAND] = COMMAND_TE | COMMAND_WAM | COMMAND_TDM;
}

bool ds1386_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(&m_ram[0], m_ram_size, actual) && actual == m_ram_size;
}

bool ds1386_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(&m_ram[0], m_ram_size, actual) && actual == m_ram_size;
}

void ds1386_device::data_w(offs_t offset, uint8_t data)
{
	if (offset >= m_ram_size)
		return;

	if (offset >= 0xe)
	{
		m_ram[offset] = data;
	}
	else
	{
		switch (offset)
		{
			case 0x00: // hundredths
			case 0x03: // minutes alarm
			case 0x05: // horus alarm
			case 0x0a: // years
				m_ram[offset] = data;
				break;

			case 0x01: // seconds
			case 0x02: // minutes
			case 0x04: // hours
				m_ram[offset] = data & 0x7f;
				break;

			case 0x06: // days
				m_ram[offset] = data & 0x07;
				break;

			case 0x07: // days alarm
				m_ram[offset] = data & 0x87;
				break;

			case 0x08: // date
				m_ram[offset] = data & 0x3f;
				break;

			case 0x09: // months
			{
				uint8_t old_value = m_ram[offset];
				m_ram[offset] = data & 0xdf;
				uint8_t changed = old_value ^ data;
				if (changed & DISABLE_SQW)
				{
					if (m_ram[offset] & DISABLE_SQW)
					{
						m_square_timer->adjust(attotime::never);
					}
					else
					{
						m_square_timer->adjust(attotime::from_hz(2048));
					}
				}
				if (changed & DISABLE_OSC)
				{
					if (m_ram[offset] & DISABLE_OSC)
					{
						m_clock_timer->adjust(attotime::never);
					}
					else
					{
						m_clock_timer->adjust(attotime::from_hz(100));
					}
				}
				break;
			}

			case 0x0c: // watchdog hundredths
			case 0x0d: // watchdog seconds
			{
				if ((m_ram[REGISTER_COMMAND] & COMMAND_WAF) != 0 && (m_ram[REGISTER_COMMAND] & COMMAND_WAM) == 0)
				{
					m_ram[REGISTER_COMMAND] &= ~COMMAND_WAF;
					m_watchdog_alarm = 0;
					if (m_ram[REGISTER_COMMAND] & COMMAND_IPSW)
						safe_intb_cb(0);
					else
						safe_inta_cb(0);
				}
				m_ram[offset] = data;
				uint8_t wd_hundredths = m_ram[REGISTER_WATCHDOG_HUNDREDTHS];
				uint8_t wd_seconds = m_ram[REGISTER_WATCHDOG_SECONDS];
				int total_hundredths = (wd_hundredths & 0xf) + ((wd_hundredths >> 4) & 0xf) * 10 + (wd_seconds & 0xf) * 100 + ((wd_seconds >> 4) & 0xf) * 1000;
				m_watchdog_timer->adjust(attotime::from_msec(total_hundredths * 10));
				break;
			}

			case 0x0b: // command
			{
				uint8_t old = m_ram[offset];
				m_ram[offset] = data & 0xfc;
				uint8_t changed = old ^ m_ram[offset];
				if (changed & COMMAND_IPSW)
				{
					int old_a = (old & COMMAND_IPSW) ? m_tod_alarm : m_watchdog_alarm;
					int old_b = (old & COMMAND_IPSW) ? m_watchdog_alarm : m_tod_alarm;
					int new_a = old_b;
					int new_b = old_a;

					bool a_changed = old_a != new_a;
					bool b_changed = old_b != new_b;

					std::swap(m_tod_alarm, m_watchdog_alarm);

					if (a_changed)
					{
						if (m_ram[offset] & COMMAND_IPSW)
							safe_inta_cb(m_tod_alarm);
						else
							safe_inta_cb(m_watchdog_alarm);
					}
					if (b_changed)
					{
						if (m_ram[offset] & COMMAND_IPSW)
							safe_intb_cb(m_watchdog_alarm);
						else
							safe_intb_cb(m_tod_alarm);
					}
				}
				if (changed & COMMAND_WAM)
				{
					if (m_ram[offset] & COMMAND_WAF)
					{
						if (m_ram[offset] & COMMAND_IPSW)
							safe_intb_cb(0);
						else
							safe_inta_cb(0);
					}
					else if (m_watchdog_alarm)
					{
						if (m_ram[offset] & COMMAND_IPSW)
							safe_intb_cb(m_watchdog_alarm);
						else
							safe_inta_cb(m_watchdog_alarm);
					}
				}
				if (changed & COMMAND_TDM)
				{
					if (m_ram[offset] & COMMAND_TDF)
					{
						if (m_ram[offset] & COMMAND_IPSW)
							safe_inta_cb(0);
						else
							safe_intb_cb(0);
					}
					else if (m_tod_alarm)
					{
						if (m_ram[offset] & COMMAND_IPSW)
							safe_inta_cb(m_tod_alarm);
						else
							safe_intb_cb(m_tod_alarm);
					}
				}
				break;
			}
		}
	}
}

uint8_t ds1386_device::data_r(offs_t offset)
{
	if (offset >= m_ram_size)
		return 0;

	return m_ram[offset];
}
