// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Mostek MK3835N/MK3831N CMOS Microcomputer Clock/RAM emulation

****************************************************************************/

#include "emu.h"
#include "mk3835.h"


DEFINE_DEVICE_TYPE(MK3835, mk3835_device, "mk3835", "Mostek MK3835 Clock/RAM")

mk3835_device::mk3835_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MK3835, tag, owner, clock)
	, device_rtc_interface(mconfig, *this)
	, device_nvram_interface(mconfig, *this)
	, m_clock_timer(nullptr)
	, m_clock{}
	, m_ram{}
	, m_ce(1)
	, m_sclk(0)
	, m_io_in(0)
	, m_io_out(1)
	, m_shift(0)
	, m_bits(0)
	, m_address(0)
	, m_ram_selected(false)
	, m_reading(false)
	, m_command_received(false)
	, m_burst(false)
	, m_transfer_done(false)
	, m_time_initialized(false)
{
	m_clock[0] = 0x80;
	m_clock[7] = 0x80;
}

void mk3835_device::device_start()
{
	m_clock_timer = timer_alloc(FUNC(mk3835_device::clock_tick), this);
	m_clock_timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));

	save_item(NAME(m_clock));
	save_item(NAME(m_ram));
	save_item(NAME(m_ce));
	save_item(NAME(m_sclk));
	save_item(NAME(m_io_in));
	save_item(NAME(m_io_out));
	save_item(NAME(m_shift));
	save_item(NAME(m_bits));
	save_item(NAME(m_address));
	save_item(NAME(m_ram_selected));
	save_item(NAME(m_reading));
	save_item(NAME(m_command_received));
	save_item(NAME(m_burst));
	save_item(NAME(m_transfer_done));
	save_item(NAME(m_register));
	save_item(NAME(m_time_initialized));
}

void mk3835_device::nvram_default()
{
	m_ram.fill(0);
}

bool mk3835_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, m_ram.data(), m_ram.size());
	if (err || actual != m_ram.size())
		return false;

	std::array<u8, 8> clock;
	auto const [clock_err, clock_actual] = read(file, clock.data(), clock.size());
	if (clock_err || (clock_actual && clock_actual != clock.size()))
		return false;
	if (clock_actual)
	{
		m_clock = clock;
		m_time_initialized = true;
		restore_clock();
	}
	return true;
}

bool mk3835_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = write(file, m_ram.data(), m_ram.size());
	if (err || actual != m_ram.size())
		return false;
	auto const [clock_err, clock_actual] = write(file, m_clock.data(), m_clock.size());
	return !clock_err && clock_actual == m_clock.size();
}

void mk3835_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	if (m_time_initialized && BIT(m_clock[0], 7))
	{
		restore_clock();
		return;
	}
	m_time_initialized = true;
	m_clock[0] = (m_clock[0] & 0x80) | convert_to_bcd(second);
	m_clock[1] = convert_to_bcd(minute);
	m_clock[2] = BIT(m_clock[2], 7)
		? 0x80 | (hour >= 12 ? 0x20 : 0) | convert_to_bcd(hour % 12 ? hour % 12 : 12)
		: convert_to_bcd(hour);
	m_clock[3] = (m_clock[3] & 0x80) | convert_to_bcd(day);
	m_clock[4] = convert_to_bcd(month);
	m_clock[5] = (m_clock[5] & 0x80) | convert_to_bcd(day_of_week);
	m_clock[6] = convert_to_bcd(year % 100);
}

void mk3835_device::restore_clock()
{
	const int hour = BIT(m_clock[2], 7)
		? bcd_to_integer(m_clock[2] & 0x1f) % 12 + (BIT(m_clock[2], 5) ? 12 : 0)
		: bcd_to_integer(m_clock[2] & 0x3f);
	set_time(false, bcd_to_integer(m_clock[6]), bcd_to_integer(m_clock[4]),
		bcd_to_integer(m_clock[3] & 0x3f), m_clock[5] & 7, hour,
		bcd_to_integer(m_clock[1]), bcd_to_integer(m_clock[0] & 0x7f));
}

TIMER_CALLBACK_MEMBER(mk3835_device::clock_tick)
{
	if (BIT(m_clock[0], 7))
		return;

	if (++m_register[RTC_SECOND] >= 60)
	{
		m_register[RTC_SECOND] = 0;
		if (++m_register[RTC_MINUTE] >= 60)
		{
			m_register[RTC_MINUTE] = 0;
			if (++m_register[RTC_HOUR] >= 24)
			{
				m_register[RTC_HOUR] = 0;
				m_register[RTC_DAY_OF_WEEK] = m_register[RTC_DAY_OF_WEEK] % 7 + 1;
				static constexpr int month_days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
				const int month = std::clamp(m_register[RTC_MONTH], 1, 12);
				const int days = month_days[month - 1] + ((month == 2 && !(m_register[RTC_YEAR] % 4)) ? 1 : 0);
				if (++m_register[RTC_DAY] > days)
				{
					m_register[RTC_DAY] = 1;
					if (++m_register[RTC_MONTH] > 12)
					{
						m_register[RTC_MONTH] = 1;
						m_register[RTC_YEAR] = (m_register[RTC_YEAR] + 1) % 100;
					}
				}
			}
		}
	}
	clock_updated();
}

void mk3835_device::ce_w(int state)
{
	const u8 new_state = state ? 1 : 0;
	if (new_state == m_ce)
		return;

	m_ce = new_state;
	m_shift = 0;
	m_bits = 0;
	m_reading = false;
	m_command_received = false;
	m_burst = false;
	m_transfer_done = false;
	m_io_out = 1;
}

void mk3835_device::io_w(int state)
{
	m_io_in = state ? 1 : 0;
}

void mk3835_device::sclk_w(int state)
{
	const u8 new_state = state ? 1 : 0;
	if (new_state == m_sclk)
		return;

	if (!m_ce && !m_transfer_done)
	{
		if (!m_sclk && new_state && !m_reading)
			receive_bit();
		else if (m_sclk && !new_state && m_reading)
			advance_output();
	}

	m_sclk = new_state;
}

void mk3835_device::receive_bit()
{
	m_shift |= m_io_in << m_bits++;
	if (m_bits != 8)
		return;

	if (m_command_received)
		write_byte(m_shift);
	else
		begin_command();

	if (!m_reading)
		m_shift = 0;
	m_bits = 0;
}

void mk3835_device::begin_command()
{
	if (!BIT(m_shift, 7))
	{
		m_transfer_done = true;
		return;
	}

	m_command_received = true;
	m_ram_selected = BIT(m_shift, 6);
	m_address = (m_shift >> 1) & 0x1f;
	m_burst = m_address == 31;
	if (m_burst)
		m_address = 0;
	m_reading = BIT(m_shift, 0);

	if (m_reading)
	{
		m_shift = read_byte();
		m_bits = 0;
	}
}

void mk3835_device::advance_output()
{
	if (m_bits == 8)
	{
		if (m_burst)
			m_address = (m_address + 1) % (m_ram_selected ? m_ram.size() : m_clock.size());
		m_shift = read_byte();
		m_bits = 0;
	}

	m_io_out = BIT(m_shift, m_bits++);
}

void mk3835_device::write_byte(u8 data)
{
	if (BIT(m_clock[7], 7))
	{
		if (!m_ram_selected && m_address == 7)
			m_clock[7] = (m_clock[7] & 0x7f) | (data & 0x80);
	}
	else if (m_ram_selected)
	{
		if (m_address < m_ram.size())
			m_ram[m_address] = data;
	}
	else if (m_address < m_clock.size())
	{
		static constexpr u8 masks[] = { 0xff, 0x7f, 0xbf, 0xbf, 0x1f, 0x87, 0xff, 0xff };
		static constexpr int registers[] = { RTC_SECOND, RTC_MINUTE, RTC_HOUR, RTC_DAY, RTC_MONTH, RTC_DAY_OF_WEEK, RTC_YEAR };
		m_clock[m_address] = data & masks[m_address];
		if (m_address < 7)
		{
			int value = bcd_to_integer(m_clock[m_address] & (m_address == 6 ? 0xff : 0x7f));
			if (m_address == 2 && BIT(data, 7))
				value = bcd_to_integer(data & 0x1f) % 12 + (BIT(data, 5) ? 12 : 0);
			set_clock_register(registers[m_address], value);
			if (m_address == 0)
				m_clock_timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));
		}
	}

	m_transfer_done = !m_burst || ++m_address >= (m_ram_selected ? m_ram.size() : m_clock.size());
}

u8 mk3835_device::read_byte() const
{
	if (m_ram_selected)
		return (m_address < m_ram.size()) ? m_ram[m_address] : 0xff;

	return (m_address < m_clock.size()) ? m_clock[m_address] : 0xff;
}
