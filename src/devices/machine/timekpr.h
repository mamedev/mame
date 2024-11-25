// license:BSD-3-Clause
// copyright-holders:Aaron Giles,smf
/***************************************************************************

    timekpr.h

    Various ST Microelectronics timekeeper SRAM implementations:
        - M48T02
        - M48T35
        - M48T37
        - M48T58
        - MK48T08
        - MK48T12

    Dallas clones that have the same functional interface:
        - DS1643

***************************************************************************/

#ifndef MAME_MACHINE_TIMEKPR_H
#define MAME_MACHINE_TIMEKPR_H

#pragma once

#include "dirtc.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> timekeeper_device

class timekeeper_device : public device_t, public device_nvram_interface, public device_rtc_interface
{
public:
	void write(offs_t offset, u8 data);
	u8 read(offs_t offset);
	void watchdog_write(u8 data = 0);

	auto reset_cb() { return m_reset_cb.bind(); }
	auto irq_cb() { return m_irq_cb.bind(); }

protected:
	// construction/destruction
	timekeeper_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 size);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	// device_rtc_interface overrides
	virtual bool rtc_feature_y2k() const override { return m_offset_century != -1; }
	virtual bool rtc_feature_leap_year() const override { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	TIMER_CALLBACK_MEMBER(timer_tick);
	TIMER_CALLBACK_MEMBER(watchdog_callback);
	devcb_write_line m_reset_cb;
	devcb_write_line m_irq_cb;

private:
	void counters_to_ram();
	void counters_from_ram();

	// internal state
	u8 m_control;
	u8 m_seconds;
	u8 m_minutes;
	u8 m_hours;
	u8 m_day;
	u8 m_date;
	u8 m_month;
	u8 m_year;
	u8 m_century;

	std::vector<u8> m_data;
	optional_region_ptr<u8> m_default_data;

	emu_timer* m_watchdog_timer;
	attotime m_watchdog_delay;
protected:
	u32 const m_size;
	s32 m_offset_watchdog;
	s32 m_offset_control;
	s32 m_offset_seconds;
	s32 m_offset_minutes;
	s32 m_offset_hours;
	s32 m_offset_day;
	s32 m_offset_date;
	s32 m_offset_month;
	s32 m_offset_year;
	s32 m_offset_century;
	s32 m_offset_flags;
};

class m48t02_device : public timekeeper_device
{
public:
	m48t02_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class m48t35_device : public timekeeper_device
{
public:
	m48t35_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class m48t37_device : public timekeeper_device
{
public:
	m48t37_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class m48t58_device : public timekeeper_device
{
public:
	m48t58_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	m48t58_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
};

class mk48t08_device : public timekeeper_device
{
public:
	mk48t08_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class mk48t12_device : public timekeeper_device
{
public:
	mk48t12_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class ds1643_device : public m48t58_device
{
public:
	ds1643_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

// device type definition
DECLARE_DEVICE_TYPE(M48T02,  m48t02_device)
DECLARE_DEVICE_TYPE(M48T35,  m48t35_device)
DECLARE_DEVICE_TYPE(M48T37,  m48t37_device)
DECLARE_DEVICE_TYPE(M48T58,  m48t58_device)
DECLARE_DEVICE_TYPE(MK48T08, mk48t08_device)
DECLARE_DEVICE_TYPE(MK48T12, mk48t12_device)
DECLARE_DEVICE_TYPE(DS1643,  ds1643_device)

#endif // MAME_MACHINE_TIMEKPR_H
