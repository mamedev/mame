// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    mc146818.h

    Implementation of the MC146818 chip

    Real time clock chip with CMOS battery backed ram
    Used in IBM PC/AT, several PC clones, Amstrad NC200, Apollo workstations

*********************************************************************/

#ifndef MAME_MACHINE_MC146818_H
#define MAME_MACHINE_MC146818_H

#pragma once

#include "dirtc.h"


class mc146818_device : public device_t,
						public device_nvram_interface,
						public device_rtc_interface
{
public:
	// construction/destruction
	mc146818_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto irq() { return m_write_irq.bind(); }
	auto sqw() { return m_write_sqw.bind(); }

	// The MC146818 doesn't have century support (some variants do), but when syncing the date & time at startup we can optionally store the century.
	void set_century_index(int century_index) { assert(!century_count_enabled()); m_century_index = century_index; }

	void set_binary(bool binary) { m_binary = binary; }
	void set_24hrs(bool hour) { m_hour = hour; }
	void set_epoch(int epoch) { m_epoch = epoch; }

	// read/write access
	void address_w(uint8_t data);
	uint8_t data_r();
	void data_w(uint8_t data);

	// direct-mapped read/write access
	uint8_t read_direct(offs_t offset);
	void write_direct(offs_t offset, uint8_t data);

	// FIXME: Addresses are read-only on a standard MC146818. Do some chipsets permit readback?
	uint8_t get_address() const { return m_index; }

protected:
	mc146818_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	// device_rtc_interface overrides
	virtual bool rtc_feature_y2k() const override { return m_epoch != 0 || m_century_index >= 0; }
	virtual bool rtc_feature_leap_year() const override { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	static constexpr unsigned char ALARM_DONTCARE = 0xc0;
	static constexpr unsigned char HOURS_PM = 0x80;

	virtual int data_size() const { return 64; }
	virtual int data_logical_size() const { return data_size(); }
	virtual bool century_count_enabled() const { return false; }

	virtual void internal_set_address(uint8_t address);
	virtual uint8_t internal_read(offs_t offset);
	virtual void internal_write(offs_t offset, uint8_t data);

	TIMER_CALLBACK_MEMBER(periodic_tick);
	TIMER_CALLBACK_MEMBER(clock_tick);
	TIMER_CALLBACK_MEMBER(time_tick);

	enum
	{
		REG_SECONDS = 0,
		REG_ALARM_SECONDS = 1,
		REG_MINUTES = 2,
		REG_ALARM_MINUTES = 3,
		REG_HOURS = 4,
		REG_ALARM_HOURS = 5,
		REG_DAYOFWEEK = 6,
		REG_DAYOFMONTH = 7,
		REG_MONTH = 8,
		REG_YEAR = 9,
		REG_A = 0xa,
		REG_B = 0xb,
		REG_C = 0xc,
		REG_D = 0xd
	};

	enum
	{
		REG_A_RS0 = 1,
		REG_A_RS1 = 2,
		REG_A_RS2 = 4,
		REG_A_RS3 = 8,
		REG_A_DV0 = 16,
		REG_A_DV1 = 32,
		REG_A_DV2 = 64,
		REG_A_UIP = 128
	};

	enum
	{
		REG_B_DSE = 1, // TODO: When set the chip will adjust the clock by an hour at start and end of DST
		REG_B_24_12 = 2,
		REG_B_DM = 4,
		REG_B_SQWE = 8, // TODO: When set the chip will output a square wave on SQW pin
		REG_B_UIE = 16,
		REG_B_AIE = 32,
		REG_B_PIE = 64,
		REG_B_SET = 128
	};

	enum
	{
		REG_C_UF = 16,
		REG_C_AF = 32,
		REG_C_PF = 64,
		REG_C_IRQF = 128
	};

	enum
	{
		REG_D_VRT = 128
	};

	// internal helpers
	int to_ram(int a) const;
	int from_ram(int a) const;
	void update_irq();
	void update_timer();
	virtual int get_timer_bypass() const;
	int get_seconds() const;
	void set_seconds(int seconds);
	int get_minutes() const;
	void set_minutes(int minutes);
	int get_hours() const;
	void set_hours(int hours);
	int get_dayofweek() const;
	void set_dayofweek(int dayofweek);
	int get_dayofmonth() const;
	void set_dayofmonth(int dayofmonth);
	int get_month() const;
	void set_month(int month);
	int get_year() const;
	void set_year(int year);
	int get_century() const;
	void set_century(int year);

	optional_memory_region m_region;

	// internal state

	uint8_t           m_index;
	std::unique_ptr<uint8_t[]> m_data;

	emu_timer *m_clock_timer;
	emu_timer *m_update_timer;
	emu_timer *m_periodic_timer;

	devcb_write_line m_write_irq;
	devcb_write_line m_write_sqw;
	int m_century_index, m_epoch;
	bool m_binary, m_hour;
	bool m_sqw_state;
	unsigned m_tuc; // update cycle time
};

class ds1287_device : public mc146818_device
{
public:
	ds1287_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class ds1397_device : public mc146818_device
{
public:
	ds1397_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	u8 xram_r(offs_t offset);
	void xram_w(offs_t offset, u8 data);

protected:
	virtual int data_size() const override { return 64 + 4096; }

	u8 m_xram_page;
};

// device type definition
DECLARE_DEVICE_TYPE(MC146818, mc146818_device)
DECLARE_DEVICE_TYPE(DS1287, ds1287_device)
DECLARE_DEVICE_TYPE(DS1397, ds1397_device)

#endif // MAME_MACHINE_MC146818_H
