// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    rtc4513.h - Epson RTC-4513 real-time clock emulation

***************************************************************************/

#ifndef MAME_MACHINE_RTC4513_H
#define MAME_MACHINE_RTC4513_H

#pragma once

#include "dirtc.h"


class rtc4513_device : public device_t,
                       public device_nvram_interface,
                       public device_rtc_interface
{
public:
	// construction/destruction
	rtc4513_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	int data_r() { return m_data; }
	void data_w(int state) { m_data = state; }
	void clk_w(int state);
	void ce_w(int state);

	auto irq_cb() { return m_irq_cb.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	// device_rtc_interface overrides
	virtual bool rtc_feature_y2k() const override { return false; }
	virtual bool rtc_feature_leap_year() const override { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	TIMER_CALLBACK_MEMBER(second_tick);
	TIMER_CALLBACK_MEMBER(periodic_tick);

	devcb_write_line m_irq_cb;

private:
	enum reg_t : u8
	{
		REG_SECONDS_L,
		REG_SECONDS_H,
		REG_MINUTES_L,
		REG_MINUTES_H,
		REG_HOURS_L,
		REG_HOURS_H,
		REG_DAY_L,
		REG_DAY_H,
		REG_MONTH_L,
		REG_MONTH_H,
		REG_YEAR_L,
		REG_YEAR_H,
		REG_WEEKDAY,
		REG_CD,
		REG_CE,
		REG_CF
	};

	enum regbit_t : u8
	{
		SECONDS_MASK = 0x7f,
		SECONDS_H_MASK = 0x07,

		MINUTES_MASK = 0x7f,
		MINUTES_H_MASK = 0x07,

		HOURS_MASK = 0x3f,
		HOURS_H_MASK = 0x03,

		DAY_MASK = 0x3f,
		DAY_H_MASK = 0x03,

		MONTH_MASK = 0x1f,
		MONTH_H_MASK = 0x01,

		WEEKDAY_MASK = 0x07,

		OSC_FLAG_BIT = 3,
		OSC_FLAG_MASK = 0x08,

		READ_FLAG_BIT = 3,
		READ_FLAG_MASK = 0x08,

		AM_PM_BIT = 2,
		AM_PM_MASK = 0x04,

		DAY_RAM_MASK = 0x04,
		MONTH_RAM_MASK = 0x06,

		ADJ30_BIT = 3,
		ADJ30_MASK = 0x08,

		IRQ_FLAG_BIT = 2,
		IRQ_FLAG_MASK = 0x04,

		CAL_HW_BIT = 1,
		CAL_HW_MASK = 0x02,

		HOLD_BIT = 0,
		HOLD_MASK = 0x01,

		INT_MODE_BIT = 2,
		INT_MODE_WIDTH = 2,
		INT_MODE_64HZ = 0,
		INT_MODE_SEC = 1,
		INT_MODE_MIN = 2,
		INT_MODE_HOUR = 3,

		INT_STND_BIT = 1,
		INT_STND_MASK = 0x02,

		INT_MASK_BIT = 0,
		INT_MASK_MASK = 0x01,

		TEST_BIT = 3,
		TEST_MASK = 0x08,

		HOUR24_BIT = 2,
		HOUR24_MASK = 0x04,

		STOP_BIT = 1,
		STOP_MASK = 0x02,

		RESET_BIT = 0,
		RESET_MASK = 0x01
	};

	enum mode_t : u8
	{
		MODE_IDLE,
		MODE_CMD,
		MODE_WRITE_ADDR,
		MODE_WRITE_DATA,
		MODE_READ_ADDR,
		MODE_READ_DATA,
	};

	enum cmd_t : u8
	{
		CMD_READ = 0x0c,
		CMD_WRITE = 0x03
	};

	void set_defaults();
	void increment_minutes();
	u8 read_register(offs_t offset);
	void write_register(offs_t offset, u8 data);

	optional_region_ptr<u8> m_default_data;

	emu_timer *m_periodic_timer;

	u8 m_r[16];
	u8 m_mode;
	u8 m_index;
	u8 m_shifter;
	u8 m_shift_count;
	bool m_data;
	bool m_clk;
	bool m_ce;
	bool m_tick_held;
};

DECLARE_DEVICE_TYPE(RTC4513,  rtc4513_device)

#endif // MAME_MACHINE_RTC4513_H
