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

	static constexpr feature_type imperfect_features() { return feature::TIMING; }

	auto in_port() { return m_in_port_cb.bind(); }
	auto out_port() { return m_out_port_cb.bind(); }

protected:
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void execute_set_input(int irqline, int state) override;

private:
	void onboard_peripherals(address_map &map) ATTR_COLD;

	u32 in_port_r();
	void out_port_w(u8 data);
	void timer_rate_w(offs_t offset, u8 data);
	u8 irq_pending_r();
	void fiq_enable_w(u8 data);
	void irq_enable_w(u8 data);

	void bus_sizing_w(u8 data);

	template <unsigned N> TIMER_CALLBACK_MEMBER(timer_irq);
	TIMER_CALLBACK_MEMBER(check_fiq);

	devcb_read32 m_in_port_cb;
	devcb_write8 m_out_port_cb;

	emu_timer *m_irq_timers[2];

	u8 m_ext_fiq;
	u8 m_ext_irq;
	u8 m_fiq_enable;
	u8 m_irq_enable;
	u8 m_irq_pending;
};


DECLARE_DEVICE_TYPE(IGS027A, igs027a_cpu_device)

#endif // MAME_IGS_IGS027A_H
