// license:BSD-3-Clause
// copyright-holders:XingXing, Vas Crabb
#ifndef MAME_IGS_IGS027A_H
#define MAME_IGS_IGS027A_H

#pragma once

#include "cpu/arm7/arm7.h"


class igs027a_cpu_device : public arm7_cpu_device
{
public:
	igs027a_cpu_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
	virtual ~igs027a_cpu_device();

	void trigger_irq(unsigned num);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void onboard_peripherals(address_map &map) ATTR_COLD;

	template <unsigned N> void timer_rate_w(u8 data);
	u8 irq_pending_r();
	void irq_enable_w(u8 data);

	void bus_sizing_w(u8 data);

	template <unsigned N> TIMER_CALLBACK_MEMBER(timer_irq);

	emu_timer *m_irq_timers[2];

	u8 m_irq_enable;
	u8 m_irq_pending;
};


DECLARE_DEVICE_TYPE(IGS027A, igs027a_cpu_device)

#endif // MAME_IGS_IGS027A_H
