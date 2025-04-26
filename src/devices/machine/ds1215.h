// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_DS1215_H
#define MAME_MACHINE_DS1215_H

#pragma once

#include "dirtc.h"

class ds1215_device_base
	: public device_t
	, public device_nvram_interface
	, public device_rtc_interface
{
public:
	auto ceo() { return m_ceo.bind(); }

	bool ceo_r() { return !m_ceo_state; }

protected:
	ds1215_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_nvram_interface implementation
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;
	virtual void nvram_default() override;

	// device_rtc_interface implementation
	virtual bool rtc_feature_y2k() const override { return false; }
	virtual bool rtc_feature_leap_year() const override { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	u8 read_bit();
	void write_bit(u8 data);

private:
	void timer(s32 param);
	void update_ceo();

	devcb_write_line m_ceo;
	emu_timer *m_timer;

	// internal state
	u8 m_mode;
	u8 m_count;
	u8 m_reg[8];
	bool m_ceo_state;
};

class ds1215_device : public ds1215_device_base
{
public:
	ds1215_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 32'768);
	~ds1215_device() {}

	u8 read();
	void write(u8 data);
};

class ds1216e_device : public ds1215_device_base
{
public:
	ds1216e_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 32'768);
	~ds1216e_device() {}

	u8 read(offs_t offset);
};

DECLARE_DEVICE_TYPE(DS1215, ds1215_device)
DECLARE_DEVICE_TYPE(DS1216E, ds1216e_device)

#endif // MAME_MACHINE_DS1215_H
