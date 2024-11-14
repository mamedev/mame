// license:BSD-3-Clause
// copyright-holders:Angelo Salese,cam900
/***************************************************************************

    v3021.h

    EM Microelectronic-Marin SA Ultra Low Power 32kHz CMOS RTC (DIP8/SO8)

    Serial Real Time Clock

***************************************************************************/

#ifndef MAME_MACHINE_V3021_H
#define MAME_MACHINE_V3021_H

#pragma once

#include "dirtc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> v3021_device

class v3021_device : public device_t, public device_rtc_interface
{
public:
	// construction/destruction
	v3021_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 32'768);

	// parallel interface
	void write(u8 data);
	u8 read();

	// serial interface
	void cs_w(int state);
	void io_w(int state);
	int io_r();

	TIMER_CALLBACK_MEMBER(timer_callback);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_rtc_interface overrides
	virtual bool rtc_feature_y2k() const override { return false; }
	virtual bool rtc_feature_leap_year() const override { return true; }
	virtual bool rtc_battery_backed() const override { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

private:
	struct rtc_regs_t
	{
		u8 sec = 0;
		u8 min = 0;
		u8 hour = 0;
		u8 day = 1;
		u8 wday = 1;
		u8 wday_bcd = 1;
		u8 wnum = 0;
		u8 month = 1;
		u8 year = 0;
	};

	bool m_cs = false;
	bool m_io = false;
	u8 m_addr = 0;
	u8 m_data = 0;
	u8 m_ram[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	u8 m_cnt = 0;
	u8 m_mode = 0;

	rtc_regs_t m_rtc;

	void copy_clock_to_ram();
	void copy_ram_to_clock();

	emu_timer *m_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(V3021, v3021_device)

#endif // MAME_MACHINE_V3021_H
