// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_GENERALPLUS_GPL_TIMEBASE_H
#define MAME_MACHINE_GENERALPLUS_GPL_TIMEBASE_H

#pragma once

#include "machine/timer.h"

class gpl_timebase_device : public device_t
{
public:
	gpl_timebase_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto updateirqs_callback() { return m_updateirqs_cb.bind(); }

	template <u16 Timer> bool timebase_irq_flag() { return ((m_timebase_ctrl[Timer] & 0x8000) && (m_timebase_ctrl[Timer] & 0x4000)) ? true : false; }

	u16 timebasea_ctrl_r();
	u16 timebaseb_ctrl_r();
	u16 timebasec_ctrl_r();

	void timebasea_ctrl_w(u16 data);
	void timebaseb_ctrl_w(u16 data);
	void timebasec_ctrl_w(u16 data);

	void timebase_reset_w(u16 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	template <u16 Timer> TIMER_DEVICE_CALLBACK_MEMBER(timebase_cb);

	template <u16 Timer> u16 timebase_ctrl_r();
	template <u16 Timer> void timebase_ctrl_w(u16 data);


	attotime get_timer_period(int timer, int ctrlval);

	u16 m_timebase_ctrl[3];
	u16 m_timebase_reset;

	required_device_array<timer_device, 3> m_timebasetimer;

	devcb_write_line m_updateirqs_cb;
};

DECLARE_DEVICE_TYPE(GPL_TIMEBASE, gpl_timebase_device)

#endif // MAME_MACHINE_GENERALPLUS_GPL_TIMEBASE_H
