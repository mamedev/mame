// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub

#ifndef MAME_MACHINE_GLUKRS_H
#define MAME_MACHINE_GLUKRS_H

#pragma once

#include "machine/nvram.h"
#include "dirtc.h"

class glukrs_device : public device_t,
					  public device_rtc_interface
{
public:
	glukrs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 32'768);

	u8 read(offs_t address);
	void write(offs_t address, u8 data);

	TIMER_CALLBACK_MEMBER(timer_callback);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

private:
	u8 m_cmos[0x100];
	required_device<nvram_device> m_nvram;
	emu_timer *m_timer;
};

DECLARE_DEVICE_TYPE(GLUKRS, glukrs_device)
#endif // MAME_MACHINE_GLUKRS_H
