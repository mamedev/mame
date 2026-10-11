// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    ABLIC S-35390A real-time clock. Implements its serial clock/calendar and
    control-register interface, using a command byte immediately after the
    start condition with no following register-address byte.

    Clock, calendar and alarm fields use a least-significant-value-first bit
    layout while bytes are transferred most significant bit first.

    Command Byte:
        0 1 1 0 C2 C1 C0 R/W

    TODO:
    - Implement INT1/INT2 output modes and alarm matching.
    - Implement clock correction.

***************************************************************************/

#include "emu.h"
#include "s35390a.h"

#include "coreutil.h"

#include <algorithm>
#include <iterator>

#define LOG_PROTOCOL (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(S35390A, s35390a_device, "s35390a", "ABLIC S-35390A RTC")


s35390a_device::s35390a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, S35390A, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, device_rtc_interface(mconfig, *this)
	, m_data{}
	, m_transfer{}
{
}


void s35390a_device::device_start()
{
	m_clock_timer = timer_alloc(FUNC(s35390a_device::clock_tick), this);
	attotime const period = attotime::from_hz(clock() / 32'768);
	m_clock_timer->adjust(period, 0, period);

	save_item(NAME(m_data));
	save_item(NAME(m_transfer));
	save_item(NAME(m_state));
	save_item(NAME(m_command));
	save_item(NAME(m_shift));
	save_item(NAME(m_bit_count));
	save_item(NAME(m_byte_count));
	save_item(NAME(m_expected));
	save_item(NAME(m_scl));
	save_item(NAME(m_sda_in));
	save_item(NAME(m_sda_out));
	save_item(NAME(m_master_ack));
}

void s35390a_device::device_reset()
{
	m_state = STATE_IDLE;
	m_command = 0;
	m_shift = 0;
	m_bit_count = 0;
	m_byte_count = 0;
	m_expected = 0;
	m_scl = 1;
	m_sda_in = 1;
	m_sda_out = 1;
	m_master_ack = 0;
}

void s35390a_device::device_post_load()
{
	sync_clock();
}


void s35390a_device::nvram_default()
{
	std::fill(std::begin(m_data), std::end(m_data), 0);
	m_data[REG_MONTH] = 1;
	m_data[REG_DAY] = 1;
	m_data[REG_STATUS_1] = 0x01;
	m_data[REG_STATUS_2] = 0x80;
	m_data[REG_INT_1] = 0x80;
	sync_clock();
}

bool s35390a_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, m_data, sizeof(m_data));
	return !err && (actual == sizeof(m_data));
}

bool s35390a_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = write(file, m_data, sizeof(m_data));
	return !err && (actual == sizeof(m_data));
}


TIMER_CALLBACK_MEMBER(s35390a_device::clock_tick)
{
	advance_seconds();
}

bool s35390a_device::rtc_feature_y2k() const
{
	return false;
}

bool s35390a_device::rtc_feature_leap_year() const
{
	return true;
}

void s35390a_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_data[REG_YEAR] = year % 100;
	m_data[REG_MONTH] = month;
	m_data[REG_DAY] = day;
	m_data[REG_DAY_OF_WEEK] = day_of_week - 1;
	m_data[REG_HOUR] = hour;
	m_data[REG_MINUTE] = minute;
	m_data[REG_SECOND] = second;
}

void s35390a_device::sync_clock()
{
	set_time(false,
		m_data[REG_YEAR],
		m_data[REG_MONTH],
		m_data[REG_DAY],
		m_data[REG_DAY_OF_WEEK] + 1,
		m_data[REG_HOUR],
		m_data[REG_MINUTE],
		m_data[REG_SECOND]);
}


void s35390a_device::scl_w(int state)
{
	state = state ? 1 : 0;
	if (m_scl == state)
		return;

	m_scl = state;
	if (m_scl)
		rising_edge();
	else
		falling_edge();
}

void s35390a_device::sda_w(int state)
{
	state = state ? 1 : 0;
	if (m_sda_in == state)
		return;

	u8 const previous = m_sda_in;
	m_sda_in = state;

	if (m_scl && m_sda_out)
	{
		if (previous && !m_sda_in)
			bus_start();
		else if (!previous && m_sda_in)
			bus_stop();
	}
}

void s35390a_device::bus_start()
{
	LOGMASKED(LOG_PROTOCOL, "start\n");
	m_state = STATE_COMMAND;
	m_shift = 0;
	m_bit_count = 0;
	m_byte_count = 0;
	m_expected = 0;
	m_sda_out = 1;
}

void s35390a_device::bus_stop()
{
	LOGMASKED(LOG_PROTOCOL, "stop\n");
	m_state = STATE_IDLE;
	m_sda_out = 1;
}

void s35390a_device::rising_edge()
{
	switch (m_state)
	{
	case STATE_COMMAND:
	case STATE_WRITE_DATA:
		if (m_bit_count < 8)
		{
			m_shift = (m_shift << 1) | m_sda_in;
			m_bit_count++;
		}
		break;

	case STATE_READ_DATA:
		if (m_bit_count < 8)
			m_bit_count++;
		break;

	case STATE_READ_ACK:
		m_master_ack = !m_sda_in;
		break;
	}
}

void s35390a_device::falling_edge()
{
	switch (m_state)
	{
	case STATE_COMMAND:
		if (m_bit_count == 8)
		{
			if ((m_shift & 0xf0) == 0x60)
			{
				m_command = (m_shift >> 1) & 0x07;
				m_expected = command_length();
				m_sda_out = 0;
				m_state = STATE_COMMAND_ACK;
				LOGMASKED(LOG_PROTOCOL, "command %u %s\n", m_command, BIT(m_shift, 0) ? "read" : "write");
			}
			else
			{
				LOGMASKED(LOG_PROTOCOL, "unknown device code %02x\n", m_shift);
				m_state = STATE_IDLE;
				m_sda_out = 1;
			}
		}
		break;

	case STATE_COMMAND_ACK:
		m_sda_out = 1;
		m_bit_count = 0;
		m_byte_count = 0;
		if (BIT(m_shift, 0))
		{
			prepare_read();
			m_state = STATE_READ_DATA;
			m_sda_out = BIT(m_transfer[0], 7);
		}
		else
		{
			m_state = STATE_WRITE_DATA;
		}
		m_shift = 0;
		break;

	case STATE_WRITE_DATA:
		if (m_bit_count == 8)
		{
			m_transfer[m_byte_count] = m_shift;
			m_sda_out = 0;
			m_state = STATE_WRITE_ACK;
		}
		break;

	case STATE_WRITE_ACK:
		m_sda_out = 1;
		m_byte_count++;
		if (m_byte_count >= m_expected)
		{
			finish_write();
			m_state = STATE_IDLE;
		}
		else
		{
			m_shift = 0;
			m_bit_count = 0;
			m_state = STATE_WRITE_DATA;
		}
		break;

	case STATE_READ_DATA:
		if (m_bit_count == 8)
		{
			finish_read_byte();
			m_sda_out = 1;
			m_master_ack = 0;
			m_state = STATE_READ_ACK;
		}
		else
		{
			m_sda_out = BIT(m_transfer[m_byte_count], 7 - m_bit_count);
		}
		break;

	case STATE_READ_ACK:
		if (m_master_ack && (m_byte_count + 1 < m_expected))
		{
			m_byte_count++;
			m_bit_count = 0;
			m_state = STATE_READ_DATA;
			m_sda_out = BIT(m_transfer[m_byte_count], 7);
		}
		else
		{
			m_state = STATE_IDLE;
			m_sda_out = 1;
		}
		break;
	}
}


bool s35390a_device::alarm_mode() const
{
	if (m_command == 4)
		return (m_data[REG_STATUS_2] & 0xf0) == 0x20;
	else if (m_command == 5)
		return (m_data[REG_STATUS_2] & 0x0e) == 0x02;

	return false;
}

u8 s35390a_device::command_length() const
{
	switch (m_command)
	{
	case 2:
		return 7;

	case 3:
		return 3;

	case 4:
	case 5:
		return alarm_mode() ? 3 : 1;

	default:
		return 1;
	}
}

u8 s35390a_device::encode_hour() const
{
	int hour = m_data[REG_HOUR];
	bool const pm = hour >= 12;
	if (!BIT(m_data[REG_STATUS_1], 6))
		hour %= 12;

	u8 result = bitswap<8>(convert_to_bcd(hour), 0, 1, 2, 3, 4, 5, 6, 7);
	return (result & ~0x02) | (pm ? 0x02 : 0x00);
}

int s35390a_device::decode_hour(u8 data) const
{
	u8 const bcd = bitswap<8>(data, 0, 1, 2, 3, 4, 5, 6, 7) & 0x3f;
	int hour = bcd_to_integer(bcd);
	int const maximum = BIT(m_data[REG_STATUS_1], 6) ? 23 : 11;
	if (((bcd & 0x0f) > 9) || ((bcd >> 4) > 9) || (hour > maximum))
		return 0;

	if (!BIT(m_data[REG_STATUS_1], 6))
	{
		if (BIT(data, 1))
			hour += 12;
	}

	return hour;
}

void s35390a_device::prepare_read()
{
	std::fill(std::begin(m_transfer), std::end(m_transfer), 0);
	m_expected = command_length();

	switch (m_command)
	{
	case 0:
		m_transfer[0] = m_data[REG_STATUS_1] & 0x7f;
		break;

	case 1:
		m_transfer[0] = m_data[REG_STATUS_2];
		break;

	case 2:
		m_transfer[0] = bitswap<8>(convert_to_bcd(m_data[REG_YEAR]), 0, 1, 2, 3, 4, 5, 6, 7);
		m_transfer[1] = bitswap<8>(convert_to_bcd(m_data[REG_MONTH]), 0, 1, 2, 3, 4, 5, 6, 7);
		m_transfer[2] = bitswap<8>(convert_to_bcd(m_data[REG_DAY]), 0, 1, 2, 3, 4, 5, 6, 7);
		m_transfer[3] = bitswap<8>(m_data[REG_DAY_OF_WEEK], 0, 1, 2, 3, 4, 5, 6, 7);
		m_transfer[4] = encode_hour();
		m_transfer[5] = bitswap<8>(convert_to_bcd(m_data[REG_MINUTE]), 0, 1, 2, 3, 4, 5, 6, 7);
		m_transfer[6] = bitswap<8>(convert_to_bcd(m_data[REG_SECOND]), 0, 1, 2, 3, 4, 5, 6, 7);
		break;

	case 3:
		m_transfer[0] = encode_hour();
		m_transfer[1] = bitswap<8>(convert_to_bcd(m_data[REG_MINUTE]), 0, 1, 2, 3, 4, 5, 6, 7);
		m_transfer[2] = bitswap<8>(convert_to_bcd(m_data[REG_SECOND]), 0, 1, 2, 3, 4, 5, 6, 7);
		break;

	case 4:
		std::copy_n(&m_data[REG_INT_1], m_expected, m_transfer);
		break;

	case 5:
		std::copy_n(&m_data[REG_INT_2], m_expected, m_transfer);
		break;

	case 6:
		m_transfer[0] = m_data[REG_CORRECTION];
		break;

	case 7:
		m_transfer[0] = m_data[REG_FREE];
		break;
	}
}

void s35390a_device::finish_read_byte()
{
	if ((m_command == 0) && !m_byte_count)
		m_data[REG_STATUS_1] &= ~0x0f;
}

void s35390a_device::finish_write()
{
	switch (m_command)
	{
	case 0:
		if (BIT(m_transfer[0], 7))
			reset_registers(m_transfer[0]);
		else
			m_data[REG_STATUS_1] = (m_data[REG_STATUS_1] & 0x0f) | (m_transfer[0] & 0x70);
		break;

	case 1:
		m_data[REG_STATUS_2] = m_transfer[0] & 0xfe;
		break;

	case 2:
		write_time(true);
		break;

	case 3:
		write_time(false);
		break;

	case 4:
		std::copy_n(m_transfer, m_expected, &m_data[REG_INT_1]);
		break;

	case 5:
		std::copy_n(m_transfer, m_expected, &m_data[REG_INT_2]);
		break;

	case 6:
		m_data[REG_CORRECTION] = m_transfer[0];
		break;

	case 7:
		m_data[REG_FREE] = m_transfer[0];
		break;
	}
}

void s35390a_device::reset_registers(u8 status)
{
	std::fill(std::begin(m_data), std::end(m_data), 0);
	m_data[REG_MONTH] = 1;
	m_data[REG_DAY] = 1;
	m_data[REG_STATUS_1] = status & 0x70;
	sync_clock();
}

void s35390a_device::write_time(bool full)
{
	auto const decode_bcd = [](u8 bcd, int minimum, int maximum, int fallback)
	{
		int const value = ((bcd >> 4) * 10) + (bcd & 0x0f);
		return ((bcd & 0x0f) <= 9) && ((bcd >> 4) <= 9) &&
			(value >= minimum) && (value <= maximum) ? value : fallback;
	};

	u8 const offset = full ? 4 : 0;
	int year = full ? decode_bcd(bitswap<8>(m_transfer[0], 0, 1, 2, 3, 4, 5, 6, 7), 0, 99, 0) : m_data[REG_YEAR];
	int month = full ? decode_bcd(bitswap<8>(m_transfer[1], 0, 1, 2, 3, 4, 5, 6, 7) & 0x1f, 1, 12, 1) : m_data[REG_MONTH];
	int day = full ? decode_bcd(bitswap<8>(m_transfer[2], 0, 1, 2, 3, 4, 5, 6, 7) & 0x3f, 1, 31, 1) : m_data[REG_DAY];
	int const day_of_week = full ? (bitswap<8>(m_transfer[3], 0, 1, 2, 3, 4, 5, 6, 7) & 0x07) % 7 : m_data[REG_DAY_OF_WEEK];
	int const hour = decode_hour(m_transfer[offset]);
	int const minute = decode_bcd(bitswap<8>(m_transfer[offset + 1], 0, 1, 2, 3, 4, 5, 6, 7) & 0x7f, 0, 59, 0);
	int const second = decode_bcd(bitswap<8>(m_transfer[offset + 2], 0, 1, 2, 3, 4, 5, 6, 7) & 0x7f, 0, 59, 0);

	// Nonexistent dates advance to the first of the next month without changing the weekday.
	if (full && (day > gregorian_days_in_month(month, 2000 + year)))
	{
		day = 1;
		if (++month > 12)
		{
			month = 1;
			year = (year + 1) % 100;
		}
	}

	set_time(true, year, month, day, day_of_week + 1, hour, minute, second);
}
