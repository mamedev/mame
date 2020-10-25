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

// ======================> bq4874_device

class bq4847_device : public device_t,
					  public device_nvram_interface,
					  public device_rtc_interface
{
public:
	bq4847_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// The watchdog is inactive when the pin is not connected; we offer
	// a method to activate it
	void set_watchdog_active(bool active);

	auto interrupt_cb() { return m_interrupt_cb.bind(); }
	auto watchdog_cb() { return m_wdout_cb.bind(); }

	// Retrigger the watchdog
	void retrigger_watchdog();

	uint8_t read(offs_t address);
	void write(offs_t address, uint8_t data);

	DECLARE_READ_LINE_MEMBER(intrq_r);

	// Mainly used to disconnect from oscillator
	void connect_osc(bool conn);

private:
	// Inherited from device_rtc_interface
	void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;
	bool rtc_feature_y2k()  const override { return false; }
	bool rtc_feature_leap_year() const override { return true; }
	bool rtc_battery_backed() const override { return true; }

	// callback called when interrupt pin state changes (may be nullptr)
	devcb_write_line    m_interrupt_cb;

	// callback called when watchdog times out changes (may be nullptr)
	devcb_write_line    m_wdout_cb;

	// Accessible registers
	uint8_t     m_reg[16];

	// Internal clock registers
	// The clock operates on these registers and copies them to the
	// accessible registers
	uint8_t     m_intreg[16];

	TIMER_CALLBACK_MEMBER(rtc_clock_cb);
	TIMER_CALLBACK_MEMBER(rtc_periodic_cb);
	TIMER_CALLBACK_MEMBER(rtc_watchdog_cb);

	void nvram_default() override;
	void nvram_read(emu_file &file) override;
	void nvram_write(emu_file &file) override;

	void device_start() override;

	// Sanity-check BCD number
	bool valid_bcd(uint8_t value, uint8_t min, uint8_t max);

	// Check bits in register
	bool is_set(int number, uint8_t flag);

	// Increment BCD number
	bool increment_bcd(uint8_t& bcdnumber, uint8_t limit, uint8_t min);

	// Set/Reset one or more bits in the register
	void set_register(int number, uint8_t bits, bool set);

	// Copy register contents from the internal registers to SRAM
	void transfer_to_access();

	// Check matching registers of time and alarm
	bool check_match(int regint, int regalarm);

	// Check whether this register is internal
	bool is_internal_register(int regnum);

	// Advance
	bool advance_hours_bcd();
	void advance_days_bcd();

	// Timers
	emu_timer *m_clock_timer;
	emu_timer *m_periodic_timer;
	emu_timer *m_watchdog_timer;

	// Set timers
	void set_periodic_timer();
	void set_watchdog_timer(bool on);

	// Get time from system
	void get_system_time();

	// Get the delay until the next second
	int get_delay();

	// Flags
	bool m_watchdog_active;
	bool m_watchdog_asserted;
	bool m_writing;
};

DECLARE_DEVICE_TYPE(BQ4847, bq4847_device)
#endif
