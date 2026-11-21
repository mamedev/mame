// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    DS1307 64x8, Serial, I2C Real-Time Clock (Maxim Integrated Products)
**********************************************************************/

#include "emu.h"

#include "ds1307.h"

#include <algorithm>
#include <iterator>


DEFINE_DEVICE_TYPE(I2C_DS1307, i2c_ds1307_device, "i2c_ds1307", "I2C DS1307 RTC/SRAM")


i2c_ds1307_device::i2c_ds1307_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, I2C_DS1307, tag, owner, clock)
	, i2c_hle_interface(mconfig, *this, 0xd0 >> 1)
	, device_rtc_interface(mconfig, *this)
	, m_cmos{}
	, m_read_buffer{}
	, m_nvram(*this, "nvram")
{
}


u8 i2c_ds1307_device::read_data(u16 offset)
{
	offset &= 0x3f;
	return offset < 7 ? m_read_buffer[offset] : m_cmos[offset];
}

void i2c_ds1307_device::write_data(u16 offset, u8 data)
{
	static constexpr u8 MASKS[8] = { 0xff, 0x7f, 0x7f, 0x07, 0x3f, 0x1f, 0xff, 0x93 };
	offset &= 0x3f;
	m_cmos[offset] = offset < 8 ? data & MASKS[offset] : data;
	if (!offset)
	{
		// Writing seconds resets the oscillator divider, including its phase.
		attotime const period = attotime::from_ticks(32'768, clock());
		m_timer->adjust(period, 0, period);
	}
}


void i2c_ds1307_device::i2c_start()
{
	std::copy_n(m_cmos, std::size(m_read_buffer), m_read_buffer);
}

bool i2c_ds1307_device::reset_pointer_on_stop() const
{
	return false;
}

bool i2c_ds1307_device::rtc_feature_leap_year() const
{
	return true;
}

void i2c_ds1307_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_cmos[0x00] = (m_cmos[0x00] & 0x80) | convert_to_bcd(second);
	m_cmos[0x01] = convert_to_bcd(minute);
	if (BIT(m_cmos[0x02], 6))
		m_cmos[0x02] = 0x40 | (hour >= 12 ? 0x20 : 0) | convert_to_bcd(hour % 12 ? hour % 12 : 12);
	else
		m_cmos[0x02] = convert_to_bcd(hour);
	m_cmos[0x03] = convert_to_bcd(day_of_week);
	m_cmos[0x04] = convert_to_bcd(day);
	m_cmos[0x05] = convert_to_bcd(month);
	m_cmos[0x06] = convert_to_bcd(year % 100);
}

TIMER_CALLBACK_MEMBER(i2c_ds1307_device::timer_callback)
{
	if (BIT(m_cmos[0x00], 7))
		return;

	// Rebuild the calendar from the battery-backed registers so writes and
	// restored state are used by the next tick, rather than being overwritten.
	int const month = bcd_to_integer(m_cmos[0x05]);
	if (month > 12) // Calendar operation with invalid register values is undefined.
		return;
	int const hour = BIT(m_cmos[0x02], 6)
			? (bcd_to_integer(m_cmos[0x02] & 0x1f) % 12) + (BIT(m_cmos[0x02], 5) ? 12 : 0)
			: bcd_to_integer(m_cmos[0x02]);
	set_time(false, bcd_to_integer(m_cmos[0x06]), month, bcd_to_integer(m_cmos[0x04]),
			m_cmos[0x03], hour, bcd_to_integer(m_cmos[0x01]), bcd_to_integer(m_cmos[0x00]));
	advance_seconds();
}


void i2c_ds1307_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);
}

void i2c_ds1307_device::device_start()
{
	m_nvram->set_base(&m_cmos, sizeof(m_cmos));
	save_item(NAME(m_cmos));
	save_item(NAME(m_read_buffer));

	m_timer = timer_alloc(FUNC(i2c_ds1307_device::timer_callback), this);
	attotime const period = attotime::from_ticks(32'768, clock());
	m_timer->adjust(period, 0, period);
}
