// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    rtc4513.cpp - Epson RTC-4513 real-time clock emulation

***************************************************************************/

#include "emu.h"
#include "rtc4513.h"

#include "machine/timehelp.h"

#define VERBOSE (1)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(RTC4513,  rtc4513_device,  "rtc4513",  "RTC-4513 Real Time Clock Module")

rtc4513_device::rtc4513_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RTC4513, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, device_rtc_interface(mconfig, *this)
	, m_irq_cb(*this)
	, m_default_data(*this, DEVICE_SELF)
	, m_periodic_timer(nullptr)
	, m_mode(MODE_IDLE)
	, m_index(0)
	, m_shifter(0)
	, m_shift_count(0)
	, m_data(false)
	, m_clk(false)
	, m_ce(false)
	, m_tick_held(false)
{
}

void rtc4513_device::device_start()
{
	save_item(NAME(m_r));
	save_item(NAME(m_mode));
	save_item(NAME(m_index));
	save_item(NAME(m_shifter));
	save_item(NAME(m_shift_count));
	save_item(NAME(m_data));
	save_item(NAME(m_clk));
	save_item(NAME(m_ce));
	save_item(NAME(m_tick_held));

	set_defaults();

	emu_timer *timer = timer_alloc(FUNC(rtc4513_device::second_tick), this);
	timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));

	m_periodic_timer = timer_alloc(FUNC(rtc4513_device::periodic_tick), this);
	m_periodic_timer->adjust(attotime::never);
}

void rtc4513_device::device_reset()
{
}

void rtc4513_device::set_defaults()
{
	memset(&m_r[0], 0, 16);
	m_r[REG_DAY_L] = 1;
	m_r[REG_MONTH_L] = 1;
	m_r[REG_CD] = HOLD_MASK;
	m_r[REG_CE] = (INT_MODE_HOUR << INT_MODE_BIT) | INT_STND_MASK | INT_MASK_MASK;
	m_r[REG_CF] = HOUR24_MASK | STOP_MASK;
}

void rtc4513_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
}

void rtc4513_device::increment_minutes()
{
	u8 minutes = ((m_r[REG_MINUTES_H] & MINUTES_H_MASK) << 4) | m_r[REG_MINUTES_L];
	bool carry = time_helper::inc_bcd(&minutes, MINUTES_MASK, 0x00, 0x59);
	m_r[REG_MINUTES_L] = minutes & 0x0f;
	m_r[REG_MINUTES_H] = (m_r[REG_MINUTES_H] & ~MINUTES_H_MASK) | ((minutes >> 4) & MINUTES_H_MASK);
	if (!carry)
		return;

	u8 hours = ((m_r[REG_HOURS_H] & HOURS_H_MASK) << 4) | m_r[REG_HOURS_L];
	const bool hour24_mode = BIT(m_r[REG_CF], HOUR24_BIT);
	const u8 max_hours = hour24_mode ? 0x23 : 0x11;
	carry = time_helper::inc_bcd(&hours, HOURS_MASK, 0x00, max_hours);
	m_r[REG_HOURS_L] = hours & 0x0f;
	m_r[REG_HOURS_H] = (m_r[REG_HOURS_H] & ~HOURS_H_MASK) | ((hours >> 4) & HOURS_H_MASK);
	if (!carry)
		return;

	if (hour24_mode && !BIT(m_r[REG_CF], HOUR24_BIT))
	{
		m_r[REG_HOURS_H] ^= AM_PM_MASK;
		if (BIT(m_r[REG_HOURS_H], AM_PM_BIT))
			return;
	}

	u8 weekday = m_r[REG_WEEKDAY] & WEEKDAY_MASK;
	u8 day = ((m_r[REG_DAY_H] & DAY_H_MASK) << 4) | m_r[REG_DAY_L];
	u8 month = ((m_r[REG_MONTH_H] & MONTH_H_MASK) << 4) | m_r[REG_MONTH_L];
	u8 year = (m_r[REG_YEAR_H] << 4) | m_r[REG_YEAR_L];

	weekday = (weekday + 1) % 7;
	m_r[REG_WEEKDAY] = (m_r[REG_WEEKDAY] & ~WEEKDAY_MASK) | weekday;

	u8 maxdays;
	static const u8 daysinmonth[] = { 0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31 };
	maxdays = (month < 0 || month >= 12) ? 0x31 : daysinmonth[month];
	if (month == 2 && (year % 4) == 0)
		maxdays++;
	carry = time_helper::inc_bcd(&day, DAY_MASK, 0x01, maxdays);
	m_r[REG_DAY_L] = day & 0x0f;
	m_r[REG_DAY_H] = (m_r[REG_DAY_H] & ~DAY_H_MASK) | ((day >> 4) & DAY_H_MASK);

	carry = time_helper::inc_bcd(&month, MONTH_MASK, 0x01, 0x12);
	m_r[REG_MONTH_L] = month & 0x0f;
	m_r[REG_MONTH_H] = (m_r[REG_MONTH_H] & ~MONTH_H_MASK) | ((month >> 4) & MONTH_H_MASK);
	if (!carry)
		return;

	time_helper::inc_bcd(&year, 0xff, 0x00, 0x99);
	m_r[REG_YEAR_L] = year & 0x0f;
	m_r[REG_YEAR_H] = (m_r[REG_YEAR_H] >> 4) & 0x0f;
}

TIMER_CALLBACK_MEMBER(rtc4513_device::second_tick)
{
	if (BIT(m_r[REG_CF], STOP_BIT))
		return;

	if (BIT(m_r[REG_CD], HOLD_BIT))
	{
		m_tick_held = true;
		return;
	}

	u8 seconds = ((m_r[REG_SECONDS_H] & SECONDS_H_MASK) << 4) | m_r[REG_SECONDS_L];
	bool carry = time_helper::inc_bcd(&seconds, SECONDS_MASK, 0x00, 0x59);
	m_r[REG_SECONDS_L] = seconds & 0x0f;
	m_r[REG_SECONDS_H] = (m_r[REG_SECONDS_H] & ~SECONDS_H_MASK) | ((seconds >> 4) & SECONDS_H_MASK);

	if (!carry)
		return;

	if (m_ce)
	{
		m_r[REG_MINUTES_H] |= READ_FLAG_MASK;
		m_r[REG_HOURS_H] |= READ_FLAG_MASK;
		m_r[REG_DAY_H] |= READ_FLAG_MASK;
		m_r[REG_MONTH_H] |= READ_FLAG_MASK;
		m_r[REG_WEEKDAY] |= READ_FLAG_MASK;
	}

	increment_minutes();
}

TIMER_CALLBACK_MEMBER(rtc4513_device::periodic_tick)
{
	// If we're in standard IRQ mode, raise the flag.
	if (BIT(m_r[REG_CE], INT_STND_BIT))
	{
		m_r[REG_CD] |= IRQ_FLAG_MASK;
		m_irq_cb(ASSERT_LINE);
		return;
	}

	// If we're not in standard IRQ mode, clear it if we're supposed to, otherwise set it.
	if (param)
	{
		m_r[REG_CD] &= ~IRQ_FLAG_BIT;
		m_irq_cb(CLEAR_LINE);
		switch (BIT(m_r[REG_CE], INT_MODE_BIT, INT_MODE_WIDTH))
		{
		case INT_MODE_64HZ:
			m_periodic_timer->adjust(attotime::from_ticks(1, 128));
			break;
		case INT_MODE_SEC:
			m_periodic_timer->adjust(attotime::from_ticks(1, 1) - attotime::from_ticks(1, 128));
			break;
		case INT_MODE_MIN:
			m_periodic_timer->adjust(attotime::from_ticks(60, 1) - attotime::from_ticks(1, 128));
			break;
		case INT_MODE_HOUR:
			m_periodic_timer->adjust(attotime::from_ticks(3600, 1) - attotime::from_ticks(1, 128));
			break;
		}
	}
	else
	{
		m_r[REG_CD] |= IRQ_FLAG_BIT;
		m_irq_cb(ASSERT_LINE);
		m_periodic_timer->adjust(attotime::from_ticks(1, 128), 1);
	}
}

void rtc4513_device::clk_w(int state)
{
	if (m_clk == state)
		return;
	m_clk = state;

	if (!m_clk)
		return;

	if (m_mode == MODE_READ_DATA)
	{
		m_shifter >>= 1;
		m_shift_count++;
		if (m_shift_count == 4)
		{
			m_shift_count = 0;
			m_shifter = read_register(m_index);
			m_index = (m_index + 1) & 0x0f;
		}
		m_data = BIT(m_shifter, 0);
		return;
	}

	m_shifter >>= 1;
	m_shifter |= (u8)m_data << 3;
	m_shift_count++;

	if (m_shift_count == 4)
	{
		switch (m_mode)
		{
		case MODE_CMD:
			if (m_shifter == CMD_READ)
			{
				LOG("Entering read-address mode\n");
				m_mode = MODE_READ_ADDR;
			}
			else
			{
				LOG("Entering write-address mode\n");
				m_mode = MODE_WRITE_ADDR;
			}
			m_shifter = 0;
			break;
		case MODE_READ_ADDR:
			m_mode = MODE_READ_DATA;
			m_index = m_shifter;
			LOG("Entering read-data mode for index %d\n", m_index);
			m_shift_count = 0;
			m_shifter = read_register(m_index);
			m_index = (m_index + 1) & 0xf;
			m_data = BIT(m_shifter, 0);
			break;
		case MODE_WRITE_ADDR:
			m_index = m_shifter;
			m_shifter = 0;
			LOG("Entering write-data mode for index %d\n", m_index);
			m_mode = MODE_WRITE_DATA;
			break;
		case MODE_WRITE_DATA:
			LOG("Writing %x to register %d\n", m_shifter, m_index);
			write_register(m_index, m_shifter & 0x0f);
			m_shifter = 0;
			m_index = (m_index + 1) & 0x0f;
			break;
		}
		m_shift_count = 0;
	}
}

void rtc4513_device::ce_w(int state)
{
	m_ce = state;
	m_shifter = 0;
	m_shift_count = 0;

	if (m_ce)
	{
		// Raising chip select
		m_mode = MODE_CMD;
		return;
	}

	// Lowering chip select
	m_mode = MODE_IDLE;
	m_r[REG_MINUTES_H] &= ~READ_FLAG_MASK;
	m_r[REG_HOURS_H] &= ~READ_FLAG_MASK;
	m_r[REG_DAY_H] &= ~READ_FLAG_MASK;
	m_r[REG_MONTH_H] &= ~READ_FLAG_MASK;
	m_r[REG_WEEKDAY] &= ~READ_FLAG_MASK;

	if (BIT(m_r[REG_CF], RESET_BIT))
	{
		m_r[REG_CF] &= ~RESET_MASK;
		m_r[REG_SECONDS_L] = 0;
		m_r[REG_SECONDS_H] &= ~SECONDS_H_MASK;
	}
}

u8 rtc4513_device::read_register(offs_t offset)
{
	const u8 data = m_r[offset];
	LOG("read_register %d: %x\n", offset, data);
	if (offset == REG_CD)
	{
		m_r[REG_CD] &= ~IRQ_FLAG_MASK;
	}
	return data;
}

void rtc4513_device::write_register(offs_t offset, u8 data)
{
	const u8 old_data = m_r[offset];
	m_r[offset] = data;
	LOG("m_r[%d] is now %02x\n", offset, m_r[offset]);
	if (offset == REG_CD)
	{
		if (!BIT(old_data, ADJ30_BIT) && BIT(data, ADJ30_BIT))
		{
			m_r[REG_CD] &= ~ADJ30_MASK;
			if ((m_r[REG_SECONDS_H] & SECONDS_H_MASK) >= 3)
				increment_minutes();
			m_r[REG_SECONDS_L] = 0;
			m_r[REG_SECONDS_H] &= ~SECONDS_H_MASK;
		}

		if (BIT(old_data, HOLD_BIT) && !BIT(data, HOLD_BIT) && m_tick_held)
		{
			m_tick_held = false;
			second_tick(0);
		}
	}
}

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void rtc4513_device::nvram_default()
{
	if (m_default_data.found())
		memcpy(&m_r[0], m_default_data, 16);
	else
		set_defaults();
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool rtc4513_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = util::read(file, &m_r[0], 16);
	return !err && actual == 16;
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool rtc4513_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = util::write(file, &m_r[0], 16);
	return !err;
}
