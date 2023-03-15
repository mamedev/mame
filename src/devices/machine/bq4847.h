// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*
    Texas Instruments/Benchmarq BQ4847 Real-time clock

    See bq4847.cpp for details.

    Michael Zapf, April 2020
*/

#ifndef MAME_MACHINE_BQ4847_H
#define MAME_MACHINE_BQ4847_H

#pragma once

#include "dirtc.h"

class bq4847_device : public device_t,
					  public device_nvram_interface,
					  public device_rtc_interface
{
public:
	bq4847_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto wdo_handler() { return m_wdo_handler.bind(); }
	auto int_handler() { return m_int_handler.bind(); }
	auto rst_handler() { return m_rst_handler.bind(); }

	uint8_t read(offs_t address);
	void write(offs_t address, uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(write_wdi); // watchdog disabled if wdi pin is left floating

protected:
	bq4847_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 32768);

	// device_t
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;

	// device_nvram_interface
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream& file) override;
	virtual bool nvram_write(util::write_stream& file) override;

	// device_rtc_interface
	virtual bool rtc_feature_y2k() const override { return false; }
	virtual bool rtc_feature_leap_year() const override { return true; }
	virtual bool rtc_battery_backed() const override { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

private:
	optional_memory_region m_region;

	devcb_write_line    m_wdo_handler;
	devcb_write_line    m_int_handler;
	devcb_write_line    m_rst_handler;

	TIMER_CALLBACK_MEMBER(update_callback);
	TIMER_CALLBACK_MEMBER(periodic_callback);
	TIMER_CALLBACK_MEMBER(watchdog_callback);

	bool increment_bcd(uint8_t& bcdnumber, uint8_t limit, uint8_t min);

	bool check_alarm(int now, int alarm);
	bool is_clock_register(int regnum);
	bool advance_hours_bcd();
	void advance_days_bcd();
	void update_int();
	void set_wdo(int state);

	emu_timer *m_update_timer;
	emu_timer *m_periodic_timer;
	emu_timer *m_watchdog_timer;

	void set_periodic_timer();
	void set_watchdog_timer(int rst_state = 1);

	uint8_t m_userbuffer[16];
	uint8_t m_register[16];
	int m_wdo_state;
	int m_int_state;
	int m_rst_state;
	int m_wdi_state;
	bool m_writing;
};

class bq4845_device : public bq4847_device
{
public:
	bq4845_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(BQ4845, bq4845_device)
DECLARE_DEVICE_TYPE(BQ4847, bq4847_device)

#endif
