// license:BSD-3-Clause
// copyright-holders:hap
/*

  OKI MSM5001N CMOS LCD Watch IC

*/

#ifndef MAME_MACHINE_MSM5001N_H
#define MAME_MACHINE_MSM5001N_H

#pragma once

#include "dirtc.h"

class msm5001n_device : public device_t, public device_rtc_interface, public device_nvram_interface
{
public:
	msm5001n_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto write_segs() { return m_write_segs.bind(); } // COM in offset, SEG pins in data

	void d_w(int state); // D button
	void s_w(int state); // S button
	void power_w(int state);

	// set_current_time is unused here (only using dirtc for the counters)
	virtual void set_current_time(const system_time &systime) override { ; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// device_nvram_interface implementation
	virtual void nvram_default() override { ; }
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override { ; } // unused

private:
	enum mode : u8
	{
		MODE_NORMAL_HRMIN = 0
	};

	emu_timer *m_timer;

	bool m_power;
	mode m_mode;
	u8 m_counter;

	devcb_write16 m_write_segs;

	TIMER_CALLBACK_MEMBER(clock_tick);
	void write_lcd(u8 *digits, bool colon);
	void initialize();
};


DECLARE_DEVICE_TYPE(MSM5001N, msm5001n_device)

#endif // MAME_MACHINE_MSM5001N_H
