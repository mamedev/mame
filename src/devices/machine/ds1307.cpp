// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    DS1307 64x8, Serial, I2C Real-Time Clock (Maxim Integrated Products)
**********************************************************************/

#include "emu.h"

#include "ds1307.h"


DEFINE_DEVICE_TYPE(I2C_DS1307, i2c_ds1307_device, "i2c_ds1307", "I2C DS1307 RTC/SRAM")


i2c_ds1307_device::i2c_ds1307_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, I2C_DS1307, tag, owner, clock)
	, i2c_hle_interface(mconfig, *this, 0xd0 >> 1)
	, device_rtc_interface(mconfig, *this)
	, m_nvram(*this, "nvram")
{
}


u8 i2c_ds1307_device::read_data(u16 offset)
{
	return m_cmos[offset & 0x3f];
}

void i2c_ds1307_device::write_data(u16 offset, u8 data)
{
	m_cmos[offset & 0x3f] = data;
}

void i2c_ds1307_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_cmos[0x00] = convert_to_bcd(second);
	m_cmos[0x01] = convert_to_bcd(minute);
	m_cmos[0x02] = convert_to_bcd(hour);
	m_cmos[0x03] = convert_to_bcd(day_of_week);
	m_cmos[0x04] = convert_to_bcd(day);
	m_cmos[0x05] = convert_to_bcd(month);
	m_cmos[0x06] = convert_to_bcd(year % 100);
}

TIMER_CALLBACK_MEMBER(i2c_ds1307_device::timer_callback)
{
	advance_seconds();
}


void i2c_ds1307_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);
}

void i2c_ds1307_device::device_start()
{
	m_nvram->set_base(&m_cmos, sizeof(m_cmos));

	m_timer = timer_alloc(FUNC(i2c_ds1307_device::timer_callback), this);
	m_timer->adjust(attotime::from_hz(clock() / XTAL(32'768)), 0, attotime::from_hz(clock() / XTAL(32'768)));
}

void i2c_ds1307_device::device_reset()
{
}
