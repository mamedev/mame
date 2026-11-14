// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// Seiko Instruments S-35390A RTC, byte-level I2C interface at 0x30-0x37.
// Time, status and free-register subset.
// TODO: Alarm outputs, frequency output and oscillator correction.

#include "emu.h"
#include "s35390a.h"

#include <algorithm>
#include <iterator>

#define LOG_RTC (1U << 1)
#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(S35390A, s35390a_device, "s35390a", "Seiko Instruments S-35390A RTC")

s35390a_device::s35390a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, S35390A, tag, owner, clock)
	, device_rtc_interface(mconfig, *this)
	, device_nvram_interface(mconfig, *this)
{
}

void s35390a_device::device_start()
{
	nvram_default();
	set_time(true, 0, 1, 1, 1, 0, 0, 0);
	std::fill(std::begin(m_transfer), std::end(m_transfer), 0);
	m_command = m_pos = m_length = 0;
	m_selected = m_read = false;
	m_timer = timer_alloc(FUNC(s35390a_device::tick), this);
	m_timer->adjust(attotime::from_ticks(32'768, clock()), 0, attotime::from_ticks(32'768, clock()));
	save_item(NAME(m_regs));
	save_item(NAME(m_time));
	save_item(NAME(m_transfer));
	save_item(NAME(m_register));
	save_item(NAME(m_command));
	save_item(NAME(m_pos));
	save_item(NAME(m_length));
	save_item(NAME(m_selected));
	save_item(NAME(m_read));
}

void s35390a_device::acknowledge(int state)
{
	if (!state && m_read)
		m_selected = false;
}

bool s35390a_device::rtc_feature_leap_year() const
{
	return true;
}

void s35390a_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_time[0] = convert_to_bcd(year % 100);
	m_time[1] = convert_to_bcd(month);
	m_time[2] = convert_to_bcd(day);
	m_time[3] = (day_of_week - 1) % 7;
	m_time[4] = convert_to_bcd(hour);
	m_time[5] = convert_to_bcd(minute);
	m_time[6] = convert_to_bcd(second);
}

TIMER_CALLBACK_MEMBER(s35390a_device::tick)
{
	advance_seconds();
}

void s35390a_device::nvram_default()
{
	for (auto &bank : m_regs)
		std::fill(std::begin(bank), std::end(bank), 0);
	m_regs[0][0] = 0x40; // Valid, battery-backed MAME base time in 24-hour mode.
}

bool s35390a_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, &m_regs[0][0], sizeof(m_regs));
	return !err && actual == sizeof(m_regs);
}

bool s35390a_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = write(file, &m_regs[0][0], sizeof(m_regs));
	return !err;
}

void s35390a_device::reset_registers()
{
	u8 const status = m_regs[0][0] & 0x70;
	nvram_default();
	m_regs[0][0] = status;
	set_time(true, 0, 1, 1, 1, 0, 0, 0);
	m_timer->adjust(attotime::from_ticks(32'768, clock()), 0, attotime::from_ticks(32'768, clock()));
}

void s35390a_device::apply_time()
{
	u32 const base = (m_command == 3) ? 4 : 0;
	for (u32 i = 0; i < m_length; ++i)
		m_time[base + i] = reverse(m_transfer[i]);
	int hour = bcd_to_integer(m_time[4] & 0x3f);
	if (!BIT(m_regs[0][0], 6))
		hour = hour % 12 + (BIT(m_time[4], 6) ? 12 : 0);
	int const month = bcd_to_integer(m_time[1] & 0x1f), day = bcd_to_integer(m_time[2] & 0x3f);
	int const minute = bcd_to_integer(m_time[5] & 0x7f), second = bcd_to_integer(m_time[6] & 0x7f);
	// Undefined out-of-range BCD writes must not reach the generic calendar.
	if (month < 1 || month > 12 || day < 1 || day > 31 || hour > 23 || minute > 59 || second > 59)
		return;
	set_time(true, bcd_to_integer(m_time[0]), month, day, (m_time[3] & 7) % 7 + 1, hour, minute, second);
	m_timer->adjust(attotime::from_ticks(32'768, clock()), 0, attotime::from_ticks(32'768, clock()));
	LOGMASKED(LOG_RTC, "%s: time set %02x-%02x-%02x %02x:%02x:%02x\n", machine().describe_context(), m_time[0], m_time[1], m_time[2], m_time[4], m_time[5], m_time[6]);
}

u8 s35390a_device::transmit(offs_t data)
{
	if (BIT(data, 8))
	{
		m_selected = (data & 0xf0) == 0x60;
		m_read = BIT(data, 0);
		m_command = (data >> 1) & 7;
		m_pos = 0;
		m_length = (m_command == 2) ? 7 : ((m_command == 3 || m_command == 4 || m_command == 5) ? 3 : 1);
		if (!m_selected)
			return 1;
		if (m_command == 2 || m_command == 3)
		{
			u32 const base = (m_command == 3) ? 4 : 0;
			for (u32 i = 0; i < m_length; ++i)
			{
				u8 value = m_time[base + i];
				if (base + i == 4)
				{
					int const hour = bcd_to_integer(value);
					if (!BIT(m_regs[0][0], 6))
						value = convert_to_bcd(hour % 12);
					if (hour >= 12)
						value |= 0x40;
				}
				m_transfer[i] = reverse(value);
			}
		}
		else
		{
			std::copy_n(m_regs[m_command], m_length, m_transfer);
		}
		return 0;
	}
	if (!m_selected || m_read || m_pos >= m_length)
		return 1;
	m_transfer[m_pos++] = data;
	if (m_pos == m_length)
	{
		if (m_command == 2 || m_command == 3)
		{
			apply_time();
		}
		else if (m_command == 0)
		{
			m_regs[0][0] = (m_regs[0][0] & 0x0f) | (data & 0x70);
			if (BIT(data, 7))
				reset_registers();
		}
		else
		{
			std::copy_n(m_transfer, m_length, m_regs[m_command]);
		}
	}
	return 0;
}

u8 s35390a_device::receive()
{
	if (!m_selected || !m_read || m_pos >= m_length)
		return 0xff;
	u8 const result = m_transfer[m_pos];
	if (!machine().side_effects_disabled())
	{
		++m_pos;
		if (!m_command)
			m_regs[0][0] &= ~0x0d; // POC and alarm flags clear on read.
	}
	return result;
}
