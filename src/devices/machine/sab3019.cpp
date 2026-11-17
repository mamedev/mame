// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Mullard SAB3019 Clock/Timer with Serial I/O emulation

    TODO:
    - F50 mains input and automatic crystal switchover
    - FSET 128 Hz output
    - POWF power failure output

**********************************************************************/

#include "emu.h"
#include "sab3019.h"

#include <algorithm>

#define VERBOSE 0
#include "logmacro.h"


DEFINE_DEVICE_TYPE(SAB3019, sab3019_device, "sab3019", "Mullard SAB3019 Clock/Timer")


namespace {

uint8_t bcd_increment(uint8_t value)
{
	return ((value & 0x0f) >= 9) ? ((value & 0xf0) + 0x10) : (value + 1);
}

} // anonymous namespace


sab3019_device::sab3019_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SAB3019, tag, owner, clock),
	device_rtc_interface(mconfig, *this),
	m_write_comp(*this),
	m_write_sec(*this),
	m_write_min(*this),
	m_prescaler_timer(nullptr),
	m_daily(false),
	m_seconds(0),
	m_half(false),
	m_dlen(false),
	m_clb(false),
	m_data_in(true),
	m_noda(false),
	m_comp(false),
	m_bit_count(0),
	m_shift(0),
	m_load(false),
	m_read_pending(false),
	m_reading(false),
	m_read_s(false),
	m_read_addr(0),
	m_read_data(0),
	m_read_bit(0),
	m_minute_flag(false)
{
}


void sab3019_device::device_start()
{
	std::fill(std::begin(m_counter), std::end(m_counter), 0);
	std::fill(std::begin(m_register), std::end(m_register), 0);

	m_prescaler_timer = timer_alloc(FUNC(sab3019_device::prescaler_tick), this);
	attotime const period = clocks_to_attotime(clock() / 2);
	m_prescaler_timer->adjust(period, 0, period);

	save_item(NAME(m_counter));
	save_item(NAME(m_register));
	save_item(NAME(m_daily));
	save_item(NAME(m_seconds));
	save_item(NAME(m_half));
	save_item(NAME(m_dlen));
	save_item(NAME(m_clb));
	save_item(NAME(m_data_in));
	save_item(NAME(m_noda));
	save_item(NAME(m_comp));
	save_item(NAME(m_bit_count));
	save_item(NAME(m_shift));
	save_item(NAME(m_load));
	save_item(NAME(m_read_pending));
	save_item(NAME(m_reading));
	save_item(NAME(m_read_s));
	save_item(NAME(m_read_addr));
	save_item(NAME(m_read_data));
	save_item(NAME(m_read_bit));
	save_item(NAME(m_minute_flag));
}


void sab3019_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_counter[MINUTES] = convert_to_bcd(minute);
	m_counter[HOURS] = convert_to_bcd(hour);
	m_counter[DATE] = convert_to_bcd(day);
	m_counter[MONTH] = convert_to_bcd(month);
	m_seconds = second;

	update_comp();
}


void sab3019_device::reset_prescaler()
{
	attotime const period = clocks_to_attotime(clock() / 2);
	m_prescaler_timer->adjust(period, 0, period);

	if (m_half)
	{
		m_half = false;
		m_write_sec(1);
	}
}


void sab3019_device::reset_seconds()
{
	if (m_seconds >= 30)
	{
		advance_minute();
		m_write_min(1);
	}

	m_seconds = 0;
	reset_prescaler();
}


TIMER_CALLBACK_MEMBER(sab3019_device::prescaler_tick)
{
	m_half = !m_half;
	m_write_sec(!m_half);

	if (m_half)
		return;

	if (++m_seconds == 60)
	{
		m_seconds = 0;
		advance_minute();
		m_write_min(1);
	}
	else if (m_seconds == 30)
	{
		m_write_min(0);
	}
}


void sab3019_device::advance_minute()
{
	m_counter[MINUTES] = bcd_increment(m_counter[MINUTES]);

	if (m_counter[MINUTES] > 0x59)
	{
		m_counter[MINUTES] = 0;
		m_counter[HOURS] = bcd_increment(m_counter[HOURS]);

		if (m_counter[HOURS] > 0x23)
		{
			m_counter[HOURS] = 0;

			uint8_t last;
			switch (m_counter[MONTH])
			{
			case 0x02: last = 0x28; break;
			case 0x04: case 0x06: case 0x09: case 0x11: last = 0x30; break;
			default: last = 0x31; break;
			}

			if (m_counter[DATE] >= last)
			{
				m_counter[DATE] = 1;
				m_counter[MONTH] = (m_counter[MONTH] >= 0x12) ? 1 : bcd_increment(m_counter[MONTH]);
			}
			else
			{
				m_counter[DATE] = bcd_increment(m_counter[DATE]);
			}
		}
	}

	m_minute_flag = true;

	update_comp();
}


void sab3019_device::update_comp()
{
	bool const date = m_daily || m_noda || ((m_counter[DATE] == m_register[DATE]) && (m_counter[MONTH] == m_register[MONTH]));
	bool const comp = (m_counter[MINUTES] == m_register[MINUTES]) && (m_counter[HOURS] == m_register[HOURS]) && date;

	if (m_comp != comp)
	{
		m_comp = comp;
		m_write_comp(m_comp);
	}
}


uint8_t sab3019_device::read_digits(bool s, uint8_t addr) const
{
	static constexpr uint8_t DIGIT_MASK[4] = { 0x1f, 0x3f, 0x3f, 0x7f };

	return (s ? m_register[addr] : m_counter[addr]) & DIGIT_MASK[addr];
}


void sab3019_device::execute(uint8_t count, uint16_t bits)
{
	if (BIT(bits, 0))
		return;

	bool const s = BIT(bits, 1);
	uint8_t const addr = BIT(bits, 2, 2);

	switch (count)
	{
	case 4:
		LOG("TIME ADDRESS S %u address %u\n", s, addr);
		m_read_s = s;
		m_read_addr = addr;
		m_read_pending = true;
		m_minute_flag = false;
		break;

	case 11:
	{
		uint8_t const data = BIT(bits, 4, 7);
		LOG("TIME SET S %u address %u data %02x\n", s, addr, data);

		uint8_t *const target = s ? m_register : m_counter;

		switch (addr)
		{
		case MINUTES:
			target[MINUTES] = data;
			break;

		case HOURS:
		case DATE:
			target[addr] = data & 0x3f;
			break;

		case MONTH:
			if (s)
			{
				target[MONTH] = data & 0x1f;
				m_daily = BIT(data, 6);
			}
			else if (BIT(data, 6))
			{
				target[MONTH] = data & 0x1f;
			}
			else
			{
				reset_seconds();
			}
			break;
		}

		m_minute_flag = false;
		update_comp();
		break;
	}

	default:
		LOG("Invalid word length %u\n", count);
		break;
	}
}


void sab3019_device::dlen_w(int state)
{
	if (m_dlen == bool(state))
		return;

	m_dlen = state;

	if (state)
	{
		m_load = false;

		if (m_read_pending)
		{
			m_read_pending = false;
			m_reading = true;
			m_read_data = read_digits(m_read_s, m_read_addr);
			m_read_bit = 0;
			LOG("TIME READ S %u address %u data %02x\n", m_read_s, m_read_addr, m_read_data);
		}
		else
		{
			m_bit_count = 0;
			m_shift = 0;
		}
	}
	else
	{
		if (m_reading)
			m_reading = false;
		else
			m_load = true;
	}
}


void sab3019_device::clb_w(int state)
{
	if (m_clb == bool(state))
		return;

	m_clb = state;

	if (state)
	{
		if (m_dlen && !m_reading)
		{
			if (m_bit_count < 16)
				m_shift |= uint16_t(m_data_in) << m_bit_count;

			if (m_bit_count < 0xff)
				m_bit_count++;
		}
		else if (!m_dlen && m_load)
		{
			m_load = false;
			execute(m_bit_count, m_shift);
		}
	}
	else if (m_reading)
	{
		if (m_read_bit < 7)
			m_read_bit++;
	}
}


void sab3019_device::data_w(int state)
{
	m_data_in = state;
}


int sab3019_device::data_r()
{
	bool out = true;

	if (m_reading)
		out = (m_read_bit < 7) ? BIT(m_read_data, m_read_bit) : true;
	else if (!m_dlen && m_minute_flag)
		out = false;

	return out && m_data_in;
}


void sab3019_device::noda_w(int state)
{
	m_noda = state;

	update_comp();
}
