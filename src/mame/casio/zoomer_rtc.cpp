// license:BSD-3-Clause
// copyright-holders:Devin Acker

#include "emu.h"
#include "zoomer_rtc.h"

#include <algorithm>

DEFINE_DEVICE_TYPE(ZOOMER_RTC, zoomer_rtc_device, "zoomer_rtc", "Zoomer Real Time Clock")

//**************************************************************************
zoomer_rtc_device::zoomer_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ZOOMER_RTC, tag, owner, clock)
	, device_rtc_interface(mconfig, *this)
	, device_nvram_interface(mconfig, *this)
	, m_tick_cb(*this)
	, m_alarm_cb(*this)
{
}

//**************************************************************************
void zoomer_rtc_device::device_start()
{
	m_month = m_day = m_year = 0;

	m_tick_timer = timer_alloc(FUNC(zoomer_rtc_device::tick_timer), this);
	m_sec_timer = timer_alloc(FUNC(zoomer_rtc_device::sec_timer), this);

	save_item(NAME(m_data));
	save_item(NAME(m_month));
	save_item(NAME(m_day));
	save_item(NAME(m_year));
}

//**************************************************************************
void zoomer_rtc_device::device_reset()
{
}

//**************************************************************************
void zoomer_rtc_device::device_clock_changed()
{
	const attotime period = attotime::from_hz(clock() / 32768.0);
	m_tick_timer->adjust(period / TICKS_PER_SEC, 0, period / TICKS_PER_SEC);
	m_sec_timer->adjust(period, 0, period);
}

//**************************************************************************
u8 zoomer_rtc_device::read(offs_t offset)
{
	return m_data[offset & 0xf];
}

//**************************************************************************
void zoomer_rtc_device::write(offs_t offset, u8 data)
{
	offset &= 0xf;

	m_data[offset] = data;

	if (offset >= REG_DATE_LOW && offset <= REG_DATE_HIGH)
		update_date();

	if (offset >= REG_SECOND && offset <= REG_DATE_HIGH)
	{
		set_time(1, m_year, m_month, m_day, 0,
			bcd_to_integer(m_data[REG_HOUR]),
			bcd_to_integer(m_data[REG_MINUTE]),
			bcd_to_integer(m_data[REG_SECOND]));
	}
}

//**************************************************************************
void zoomer_rtc_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_data[REG_HOUR] = convert_to_bcd(hour);
	m_data[REG_MINUTE] = convert_to_bcd(minute);
	m_data[REG_SECOND] = convert_to_bcd(second);

	if (year != m_year || month != m_month || day != m_day)
	{
		struct tm date = {};
		date.tm_year = year - 1900;
		date.tm_mon  = month - 1;
		date.tm_mday = day;

		const time_t timediff = (mktime(&date) - RTC_EPOCH) / (60 * 60 * 24);
		m_data[REG_DATE_LOW]  = convert_to_bcd(timediff % 100);
		m_data[REG_DATE_MID]  = convert_to_bcd((timediff / 100) % 100);
		m_data[REG_DATE_HIGH] = convert_to_bcd((timediff / 10000) % 100);
	}
}

//**************************************************************************
void zoomer_rtc_device::update_date()
{
	const time_t date = RTC_EPOCH +
		(bcd_to_integer(m_data[REG_DATE_LOW]) +
		 bcd_to_integer(m_data[REG_DATE_MID]) * 100 +
		 bcd_to_integer(m_data[REG_DATE_HIGH]) * 10000)
		* 60 * 60 * 24;

	const struct tm *timenow = localtime(&date);
	if (timenow)
	{
		m_year  = timenow->tm_year + 1900;
		m_month = timenow->tm_mon + 1;
		m_day   = timenow->tm_mday;
	}
}

//**************************************************************************
TIMER_CALLBACK_MEMBER(zoomer_rtc_device::tick_timer)
{
	m_tick_cb(1);
	m_tick_cb(0);
}

//**************************************************************************
TIMER_CALLBACK_MEMBER(zoomer_rtc_device::sec_timer)
{
	advance_seconds();

	if (!memcmp(m_data + REG_SECOND, m_data + REG_ALARM_SECOND, REG_DATE_HIGH - REG_SECOND + 1))
		m_alarm_cb(1);
	else
		m_alarm_cb(0);
}

//**************************************************************************
void zoomer_rtc_device::nvram_default()
{
	std::fill(std::begin(m_data), std::end(m_data), 0);
	update_date();
}

//**************************************************************************
bool zoomer_rtc_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = util::read(file, m_data, sizeof(m_data));
	update_date();
	return !err;
}

//**************************************************************************
bool zoomer_rtc_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = util::write(file, m_data, sizeof(m_data));
	return !err;
}
