// license:BSD-3-Clause
// copyright-holders:Paul-Arnold
/*
 * ds1207.h
 *
 * Time Key
 *
 */

#ifndef MAME_MACHINE_DS1207_H
#define MAME_MACHINE_DS1207_H

#pragma once

#include "dirtc.h"

class ds1207_device : public device_t, public device_nvram_interface, public device_rtc_interface
{
public:
	// construction/destruction
	ds1207_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void write_rst(int state);
	void write_clk(int state);
	void write_dq(int state);
	int read_dq();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	// device_rtc_interface overrides
	virtual bool rtc_feature_y2k() const override { return true; }
	virtual bool rtc_feature_leap_year() const override { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

private:
	void new_state(uint8_t state);
	void writebit(uint8_t *buffer);
	void readbit(uint8_t *buffer);
	void set_start_time();
	void adjust_days_left();
	void adjust_time_into_day();

	enum state_t
	{
		STATE_STOP,
		STATE_PROTOCOL,
		STATE_READ_IDENTIFICATION,
		STATE_WRITE_IDENTIFICATION,
		STATE_WRITE_COMPARE_REGISTER,
		STATE_WRITE_SECURITY_MATCH,
		STATE_READ_SECURE_MEMORY,
		STATE_WRITE_SECURE_MEMORY,
		STATE_OUTPUT_GARBLED_DATA,
		STATE_READ_DAY_CLOCK,
		STATE_READ_DAYS_REMAINING,
		STATE_WRITE_DAYS_REMAINING
	};

	enum command_t
	{
		COMMAND_READ = 0x62,
		COMMAND_WRITE = 0x9d,
		COMMAND_READ_DAY_CLOCK = 0xf1,
		COMMAND_WRITE_DAYS_REMAINING = 0xf2,
		COMMAND_READ_DAYS_REMAINING = 0xf3,
		COMMAND_STOP_OSCILLATOR = 0xf4,
		COMMAND_ARM_OSCILLATOR = 0xf5,
		COMMAND_LOCK_DAYS_COUNT = 0xf6
	};

	enum cycle_t
	{
		CYCLE_NORMAL = 1,
		CYCLE_PROGRAM = 2,
		CYCLE_MASK = 3
	};

	enum device_state_t
	{
		OSC_ENABLED = 1,
		OSC_RUNNING = 2,
		DAYS_LOCKED = 4,
		DAYS_EXPIRED = 8
	};

	static const int8_t DQ_HIGH_IMPEDANCE = -1;

	optional_memory_region m_region;

	uint8_t m_rst;
	uint8_t m_clk;
	uint8_t m_dqw;
	int8_t m_dqr;
	uint8_t m_state;
	uint16_t m_bit;
	uint64_t m_startup_time;
	uint64_t m_last_update_time;
	uint8_t m_command[3];
	uint8_t m_day_clock[3];
	uint8_t m_compare_register[8];
	uint8_t m_unique_pattern[2];
	uint8_t m_identification[8];
	uint8_t m_security_match[8];
	uint8_t m_secure_memory[48];
	uint8_t m_days_left[2];
	uint8_t m_start_time[8];
	uint8_t m_device_state;
};


// device type definition
DECLARE_DEVICE_TYPE(DS1207, ds1207_device)

#endif // MAME_MACHINE_DS1207_H
