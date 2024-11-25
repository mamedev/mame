// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub

#ifndef MAME_SINCLAIR_GLUKRS_H
#define MAME_SINCLAIR_GLUKRS_H

#pragma once

#include "machine/nvram.h"
#include "dirtc.h"

class glukrs_device : public device_t,
					  public device_rtc_interface
{
public:
	glukrs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 32'768);

	void enable() { m_glukrs_active = true; }
	void disable() { m_glukrs_active = false; }
	bool is_active() { return m_glukrs_active; }
	u8 address_r() { return m_glukrs_active ? m_address : 0xff; }
	void address_w(u8 address) { if (m_glukrs_active) m_address = address; }
	u8 data_r() { return m_glukrs_active ? m_cmos[m_address] : 0xff; }
	void data_w(u8 data) { if (m_glukrs_active) { m_cmos[m_address] = data; } }

	TIMER_CALLBACK_MEMBER(timer_callback);

protected:
	void device_add_mconfig(machine_config &config) override ATTR_COLD;
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

private:
	bool m_glukrs_active;
	u8 m_address;

	u8 m_cmos[0x100];
	required_device<nvram_device> m_nvram;
	emu_timer *m_timer;
};

DECLARE_DEVICE_TYPE(GLUKRS, glukrs_device)
#endif // MAME_SINCLAIR_GLUKRS_H
