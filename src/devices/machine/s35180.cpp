// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    s35180.cpp
    Seiko S-35180 and S-3511A real-time clocks with an EEPROM-style 3-wire
	interface (chip select, clock, and bi-directional data).

    Note (yet?) emulated: the alarm interrupt outputs and the clock adjustment.

***************************************************************************/

#include "emu.h"
#include "s35180.h"


DEFINE_DEVICE_TYPE(S35180, s35180_device, "s35180", "Seiko S-35180 RTC")
DEFINE_DEVICE_TYPE(S3511, s3511_device, "s3511", "Seiko S-3511A RTC")


s35180_device::s35180_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: s35180_device(mconfig, S35180, tag, owner, clock)
{
}

s35180_device::s35180_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_rtc_interface(mconfig, *this)
	, m_year(0)
	, m_month(1)
	, m_day(1)
	, m_weekday(0)
	, m_hour(0)
	, m_minute(0)
	, m_second(0)
	, m_stat1(0x02)
	, m_stat2(0)
	, m_adjust(0)
	, m_free(0)
	, m_alarm{ { 0, 0, 0 }, { 0, 0, 0 } }
	, m_clock_timer(nullptr)
	, m_cs(0)
	, m_sck(0)
	, m_data(0)
	, m_cs_changed(false)
	, m_cmd(0)
	, m_shift(0)
	, m_bitpos(0)
	, m_bytepos(0)
	, m_outbit(0)
	, m_have_cmd(false)
	, m_reading(false)
{
}

void s35180_device::device_start()
{
	m_clock_timer = timer_alloc(FUNC(s35180_device::clock_tick), this);
	m_clock_timer->adjust(attotime::from_hz(clock() / 32768), 0, attotime::from_hz(clock() / 32768));

	save_item(NAME(m_year));
	save_item(NAME(m_month));
	save_item(NAME(m_day));
	save_item(NAME(m_weekday));
	save_item(NAME(m_hour));
	save_item(NAME(m_minute));
	save_item(NAME(m_second));
	save_item(NAME(m_stat1));
	save_item(NAME(m_stat2));
	save_item(NAME(m_adjust));
	save_item(NAME(m_free));
	save_item(NAME(m_alarm));
	save_item(NAME(m_cs));
	save_item(NAME(m_sck));
	save_item(NAME(m_data));
	save_item(NAME(m_cs_changed));
	save_item(NAME(m_cmd));
	save_item(NAME(m_shift));
	save_item(NAME(m_bitpos));
	save_item(NAME(m_bytepos));
	save_item(NAME(m_outbit));
	save_item(NAME(m_have_cmd));
	save_item(NAME(m_reading));
}

void s35180_device::device_reset()
{
	// 24 hour mode; the power-on and reset flags must stay clear or software
	// decides the clock has never been set
	m_stat1 = 0x02;
	m_stat2 = 0;
	m_adjust = 0;
	m_free = 0;
	std::fill_n(&m_alarm[0][0], 6, 0);

	reset_transaction();
	m_outbit = 0;
}

TIMER_CALLBACK_MEMBER(s35180_device::clock_tick)
{
	advance_seconds();
}

void s35180_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_year = year;
	m_month = month;
	m_day = day;
	m_weekday = day_of_week - 1;
	m_hour = hour;
	m_minute = minute;
	m_second = second;
}

void s35180_device::reset_transaction()
{
	m_have_cmd = false;
	m_bitpos = 0;
	m_bytepos = 0;
	m_shift = 0;
}

uint8_t s35180_device::hour_byte() const
{
	return convert_to_bcd(hour_24() ? m_hour : (m_hour % 12)) | ((m_hour >= 12) ? 0x40 : 0x00);
}

uint8_t s35180_device::datetime_byte(int index) const
{
	switch (index)
	{
		case 0: return convert_to_bcd(m_year);
		case 1: return convert_to_bcd(m_month);
		case 2: return convert_to_bcd(m_day);
		case 3: return m_weekday;
		case 4: return hour_byte();
		case 5: return convert_to_bcd(m_minute);
		case 6: return convert_to_bcd(m_second);
	}

	return 0;
}

uint8_t s35180_device::time_byte(int index) const
{
	switch (index)
	{
		case 0: return hour_byte();
		case 1: return convert_to_bcd(m_minute);
		case 2: return convert_to_bcd(m_second);
	}

	return 0;
}

uint8_t s35180_device::read_param(int index)
{
	switch (command_reg())
	{
		case 0:     // status register 1
			return m_stat1;

		case 4:     // status register 2
			return m_stat2;

		case 2:     // date and time
			return datetime_byte(index);

		case 6:     // time only
			return time_byte(index);

		case 1:     // INT1 / alarm 1
			return m_alarm[0][index % 3];

		case 5:     // INT2 / alarm 2
			return m_alarm[1][index % 3];

		case 3:     // clock adjustment
			return m_adjust;

		case 7:     // free register
			return m_free;
	}

	return 0;
}

void s35180_device::write_param(int index, uint8_t data)
{
	switch (command_reg())
	{
		case 0:
			// bit 0 resets the chip, bits 4-7 are status flags cleared on read
			m_stat1 = (m_stat1 & 0xf0) | (data & 0x0e);
			break;

		case 4:
			m_stat2 = data;
			break;

		case 1:
			m_alarm[0][index % 3] = data;
			break;

		case 5:
			m_alarm[1][index % 3] = data;
			break;

		case 3:
			m_adjust = data;
			break;

		case 7:
			m_free = data;
			break;

		default:
			// setting the time is not supported
			break;
	}
}

void s35180_device::begin_command()
{
	// accept the command byte either LSB first or MSB first
	if ((m_cmd & 0x0f) != 0x06)
	{
		if ((m_cmd & 0xf0) != 0x60)
		{
			logerror("%s: bad command %02x\n", machine().describe_context(), m_cmd);
			m_have_cmd = false;
			m_bitpos = 0;
			return;
		}

		m_cmd = 0x06 | (bitswap<3>(m_cmd, 1, 2, 3) << 4) | (BIT(m_cmd, 0) << 7);
	}

	m_reading = BIT(m_cmd, 7);
	m_bitpos = 0;
	m_bytepos = 0;
	m_shift = m_reading ? read_param(0) : 0;
}

void s35180_device::data_w(int state)
{
	m_data = state ? 1 : 0;
}

void s35180_device::cs_w(int state)
{
	const uint8_t newstate = state ? 1 : 0;
	if (newstate == m_cs)
	{
		return;
	}

	// changing selection state resets any in-progress transaction
	m_cs = newstate;
	m_cs_changed = true;
	reset_transaction();
}

void s35180_device::sck_w(int state)
{
	const uint8_t old = m_sck;
	m_sck = state ? 1 : 0;

	// ignore clock changes when the chip is not selected or just changed
	if (!m_cs || m_cs_changed)
	{
		m_cs_changed = false;
		return;
	}

	// falling edge: clock a bit out if outputting
	if (old && !m_sck)
	{
		if (m_have_cmd && m_reading)
		{
			m_outbit = BIT(m_shift, 0);
			m_shift >>= 1;
			if (++m_bitpos == 8)
			{
				m_bitpos = 0;
				m_shift = read_param(++m_bytepos);
			}
		}
		return;
	}

	if (old || !m_sck)
	{
		return;
	}

	// rising edge: clock a bit in if inputting
	if (!m_have_cmd)
	{
		m_shift = (m_shift >> 1) | (m_data << 7);
		if (++m_bitpos == 8)
		{
			m_cmd = m_shift;
			m_have_cmd = true;
			begin_command();
		}
		return;
	}

	if (!m_reading)
	{
		m_shift = (m_shift >> 1) | (m_data << 7);
		if (++m_bitpos == 8)
		{
			write_param(m_bytepos++, m_shift);
			m_bitpos = 0;
			m_shift = 0;
		}
	}
}


/***************************************************************************
    S-3511A section: basically compatible but only 4 registers instead of 8
***************************************************************************/

s3511_device::s3511_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: s35180_device(mconfig, S3511, tag, owner, clock)
	, m_status(0x40)
{
}

void s3511_device::device_start()
{
	s35180_device::device_start();

	save_item(NAME(m_status));
}

void s3511_device::device_reset()
{
	s35180_device::device_reset();

	m_status = 0x40;    // 24 hour mode
}

uint8_t s3511_device::hour_byte() const
{
	if (hour_24())
	{
		return convert_to_bcd(m_hour);
	}

	return convert_to_bcd(m_hour % 12) | ((m_hour >= 12) ? 0x40 : 0x00);
}

uint8_t s3511_device::read_param(int index)
{
	switch (command_reg())
	{
		case 4:     // status
			return m_status;

		case 2:     // date and time
			return datetime_byte(index);

		case 6:     // time
			return time_byte(index);
	}

	return 0;
}

void s3511_device::write_param(int index, uint8_t data)
{
	switch (command_reg())
	{
		case 0:     // reset
			m_status = 0x40;
			break;

		case 4:     // status
			m_status = data & 0x6a;
			break;

		default:
			// setting the date and time is not supported
			break;
	}
}
