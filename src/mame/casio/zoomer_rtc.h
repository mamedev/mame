// license:BSD-3-Clause
// copyright-holders:Devin Acker

#ifndef MAME_CASIO_ZOOMER_RTC_H
#define MAME_CASIO_ZOOMER_RTC_H

#pragma once

#include "dirtc.h"

class zoomer_rtc_device : public device_t,
	public device_rtc_interface,
	public device_nvram_interface
{
public:
	zoomer_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto alarm_cb() { return m_alarm_cb.bind(); }
	auto tick_cb() { return m_tick_cb.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	virtual bool rtc_feature_leap_year() const override { return true; }
	virtual bool rtc_feature_y2k() const override { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	static constexpr int TICKS_PER_SEC = 8;
	static constexpr s64 RTC_EPOCH = -2203959600LL; // 2/28/1900

	enum
	{
		REG_SECOND = 0x1,
		REG_MINUTE,
		REG_HOUR,
		REG_DATE_LOW,
		REG_DATE_MID,
		REG_DATE_HIGH,
		REG_ALARM_SECOND = 0x9,
		REG_ALARM_MINUTE,
		REG_ALARM_HOUR,
		REG_ALARM_DATE_LOW,
		REG_ALARM_DATE_MID,
		REG_ALARM_DATE_HIGH
	};

	void update_date();

	TIMER_CALLBACK_MEMBER(tick_timer);
	TIMER_CALLBACK_MEMBER(sec_timer);

	emu_timer *m_tick_timer, *m_sec_timer;
	devcb_write_line m_tick_cb, m_alarm_cb;

	u8 m_data[16];
	u8 m_day, m_month;
	u16 m_year;
};

DECLARE_DEVICE_TYPE(ZOOMER_RTC, zoomer_rtc_device)

#endif // MAME_CASIO_ZOOMER_RTC_H
