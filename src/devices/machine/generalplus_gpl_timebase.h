// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_GENERALPLUS_GPL_TIMEBASE_H
#define MAME_MACHINE_GENERALPLUS_GPL_TIMEBASE_H

#pragma once

#include "machine/timer.h"

class gpl_timebase_device : public device_t
{
public:
	gpl_timebase_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	bool timebasea_irq_flag() { return m_timebasea_ctrl & 0x8000 ? true : false; }
	bool timebaseb_irq_flag() { return m_timebaseb_ctrl & 0x8000 ? true : false; }
	bool timebasec_irq_flag() { return m_timebasec_ctrl & 0x8000 ? true : false; }

	auto updateirqs_callback() { return m_updateirqs_cb.bind(); }

	u16 timebasea_ctrl_r();
	void timebasea_ctrl_w(u16 data);

	u16 timebaseb_ctrl_r();
	void timebaseb_ctrl_w(u16 data);

	u16 timebasec_ctrl_r();
	void timebasec_ctrl_w(u16 data);

	void timebase_reset_w(u16 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	TIMER_DEVICE_CALLBACK_MEMBER(timebase_a_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(timebase_b_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(timebase_c_cb);

	u16 m_timebasea_ctrl;
	u16 m_timebaseb_ctrl;
	u16 m_timebasec_ctrl;

	u16 m_timebase_reset;

	required_device<timer_device> m_timebase_a;
	required_device<timer_device> m_timebase_b;
	required_device<timer_device> m_timebase_c;

	devcb_write_line m_updateirqs_cb;
};

DECLARE_DEVICE_TYPE(GPL_TIMEBASE, gpl_timebase_device)

#endif // MAME_MACHINE_GENERALPLUS_GPL_TIMEBASE_H
