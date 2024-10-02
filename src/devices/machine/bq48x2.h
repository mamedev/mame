// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*
    Texas Instruments/Benchmarq BQ4842/52 Real-time clock

    See bq48x2.cpp for details.

    Michael Zapf, April 2020
*/

#ifndef MAME_MACHINE_BQ48X2_H
#define MAME_MACHINE_BQ48X2_H

#pragma once

#include "dirtc.h"

// ======================> bq48x2_device

class bq48x2_device : public device_t,
					  public device_nvram_interface,
					  public device_rtc_interface
{
public:
	auto interrupt_cb() { return m_interrupt_cb.bind(); }
	auto resetout_cb() { return m_resetout_cb.bind(); }

	virtual uint8_t read(offs_t address);
	virtual void write(offs_t address, uint8_t data);

	int intrq_r();

	// Mainly used to disconnect from oscillator
	void connect_osc(bool conn);

protected:
	// construction/destruction (called from the subtypes)
	bq48x2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, int memsize);

	// callback called when interrupt pin state changes (may be nullptr)
	devcb_write_line    m_interrupt_cb;

	// callback called when watchdog times out changes (may be nullptr)
	devcb_write_line    m_resetout_cb;

	// Internal memory
	std::unique_ptr<u8 []>      m_sram;

	// Internal clock registers
	// The clock operates on these registers and copies them to the
	// registers in the address space
	uint8_t     m_intreg[16];

	TIMER_CALLBACK_MEMBER(rtc_clock_cb);
	TIMER_CALLBACK_MEMBER(rtc_periodic_cb);
	TIMER_CALLBACK_MEMBER(rtc_watchdog_cb);

	void nvram_default() override;
	bool nvram_read(util::read_stream &file) override;
	bool nvram_write(util::write_stream &file) override;

	void device_start() override ATTR_COLD;

	void set_watchdog_timer();

private:
	// Inherited from device_rtc_interface
	void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;
	bool rtc_feature_y2k()  const override { return false; }
	bool rtc_feature_leap_year() const override { return true; }
	bool rtc_battery_backed() const override { return true; }

	// Sanity-check BCD number
	bool valid_bcd(uint8_t value, uint8_t min, uint8_t max);

	// Increment BCD number
	bool increment_bcd(uint8_t& bcdnumber, uint8_t limit, uint8_t min);

	// Fetch register value from SRAM using register number
	uint8_t get_register(int number, uint8_t mask);

	// Set a register in SRAM using the register number
	void set_register(int number, uint8_t value);

	// Set/Reset one or more bits in the register
	void set_register(int number, uint8_t bits, bool set);

	// Check matching registers of time and alarm
	bool check_match(int regint, int regalarm, uint8_t mask);

	// Check bits in register
	bool is_set(int number, uint8_t flag);

	// Advance
	void advance_days_bcd();

	// Copy register contents from the internal registers to SRAM or back
	void transfer_to_int();
	void transfer_to_access();

	// clock timer: called every second
	emu_timer *m_clock_timer;

	// Periodic timer
	emu_timer *m_periodic_timer;

	// Watchdog timer
	emu_timer *m_watchdog_timer;

	// Set timers
	void set_periodic_timer();

	// Get the delay until the next second
	int get_delay();

	int m_memsize;
};

// Special types

class bq4842_device : public bq48x2_device
{
public:
	bq4842_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class bq4852_device : public bq48x2_device
{
public:
	bq4852_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(BQ4842, bq4842_device)
DECLARE_DEVICE_TYPE(BQ4852, bq4852_device)
#endif
