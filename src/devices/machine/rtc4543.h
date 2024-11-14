// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    rtc4543.h - Epson R4543 real-time clock emulation
    by R. Belmont

**********************************************************************/

#ifndef MAME_MACHINE_RTC4543_H
#define MAME_MACHINE_RTC4543_H

#pragma once

#include "dirtc.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> rtc4543_device

class rtc4543_device :  public device_t,
						public device_rtc_interface
{
	static char const *const s_reg_names[7];

public:
	// construction/destruction
	rtc4543_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void ce_w(int state);
	void wr_w(int state);
	void clk_w(int state);
	int data_r();
	void data_w(int state);

	auto data_cb() { return m_data_cb.bind(); }

protected:
	rtc4543_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_rtc_interface overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;
	virtual bool rtc_feature_leap_year() const override { return true; }

	// helpers
	virtual void ce_rising();
	virtual void ce_falling();
	virtual void clk_rising();
	virtual void clk_falling();
	void load_bit(int reg);
	void store_bit(int reg);
	void advance_bit();
	void update_effective();

	TIMER_CALLBACK_MEMBER(advance_clock);

	devcb_write_line m_data_cb;

	int m_ce;
	int m_clk;
	int m_wr;
	int m_data;
	int m_regs[7];
	int m_curbit;

	// timers
	emu_timer *m_clock_timer;
};



// ======================> jrc6355e_device

class jrc6355e_device : public rtc4543_device
{
public:
	// construction/destruction
	jrc6355e_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// rtc4543 overrides
	virtual void ce_rising() override;
	virtual void ce_falling() override;
	virtual void clk_rising() override;
	virtual void clk_falling() override;
};


// device type definition
DECLARE_DEVICE_TYPE(RTC4543,  rtc4543_device)
DECLARE_DEVICE_TYPE(JRC6355E, jrc6355e_device)

#endif // MAME_MACHINE_RTC4543_H
