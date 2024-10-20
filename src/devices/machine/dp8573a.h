// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#ifndef DEVICES_MACHINE_DP8573_H
#define DEVICES_MACHINE_DP8573_H

#pragma once

#include "dirtc.h"

class dp8573a_device : public device_t, public device_nvram_interface, public device_rtc_interface
{
public:
	dp8573a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 32'768);

	virtual void write(offs_t offset, uint8_t data);
	virtual uint8_t read(offs_t offset);
	void pfail_w(int state) {}

	auto intr() { return m_intr_cb.bind(); }
	auto mfo() { return m_mfo_cb.bind(); }

protected:
	dp8573a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_nvram_interface implementation
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	// device_rtc_interface implementation
	virtual bool rtc_feature_y2k() const override { return false; }
	virtual bool rtc_feature_leap_year() const override { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	void save_registers();
	void set_interrupt(uint8_t mask);
	void clear_interrupt(uint8_t mask);

	TIMER_CALLBACK_MEMBER(msec_tick);

	virtual unsigned const ram_size() { return 32; }

	std::unique_ptr<uint8_t[]> m_ram;
	uint8_t m_tscr;
	uint8_t m_pfr;
	uint8_t m_millis;

	emu_timer *m_timer;

	devcb_write_line m_intr_cb;
	devcb_write_line m_mfo_cb;
};

class dp8572a_device : public dp8573a_device
{
public:
	dp8572a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write(offs_t offset, uint8_t data) override;
	virtual uint8_t read(offs_t offset) override;

protected:
	virtual unsigned const ram_size() override { return 64; }
};

DECLARE_DEVICE_TYPE(DP8572A, dp8572a_device)
DECLARE_DEVICE_TYPE(DP8573A, dp8573a_device)

#endif // DEVICES_MACHINE_DP8573_H
