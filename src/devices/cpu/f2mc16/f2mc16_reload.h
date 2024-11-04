// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    Fujitsu Micro F2MC-16 series 16-bit reload timer

***************************************************************************/

#ifndef MAME_CPU_F2MC16_F2MC16_TIMER_H
#define MAME_CPU_F2MC16_F2MC16_TIMER_H

#pragma once

#include "f2mc16_intc.h"

class f2mc16_reload_timer_device :
	public device_t
{
public:
	f2mc16_reload_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, required_device<f2mc16_intc_device> &intc, uint8_t vector);
	f2mc16_reload_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto internal_hz() { return m_internal_hz_cb.bind(); }
	auto tot_hz() { return m_tot_hz_cb.bind(); }

	void tin_hz(const XTAL xtal) { tin_hz(xtal.value()); }
	void tin_hz(uint32_t hz);
	void tin(int state);
	void i2osclr(int state);

	uint16_t tmcsr_r(offs_t offset, uint16_t mem_mask);
	void tmcsr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t tmr_r();
	void tmrlr_w(offs_t offset, uint16_t data, uint16_t mem_mask);

protected:
	// device_t
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;
	virtual void device_reset() override ATTR_COLD;

	void update();
	void trigger();
	void update_tmr();
	uint16_t calculate_tmr();
	TIMER_CALLBACK_MEMBER(update_tin_hz);
	TIMER_CALLBACK_MEMBER(update_peripheral_clock);
	TIMER_CALLBACK_MEMBER(timer_callback);

	f2mc16_device *m_cpu;
	required_device<f2mc16_intc_device> m_intc;
	uint8_t m_vector;
	devcb_write32 m_internal_hz_cb;
	devcb_write32 m_tot_hz_cb;

	emu_timer *m_timer;
	attotime m_peripheral_clock_changed;
	attotime m_tin_changed;
	attotime m_start_time;
	attotime m_underflow_time;
	uint32_t m_clocksel;
	uint32_t m_peripheral_clock_hz;
	uint32_t m_tin_hz;
	uint32_t m_internal_hz;
	uint32_t m_tot_hz;
	int8_t m_tin;
	int8_t m_i2osclr;
	uint16_t m_tmcsr;
	uint16_t m_tmr;
	uint16_t m_tmrlr;
};

DECLARE_DEVICE_TYPE(F2MC16_RELOAD_TIMER, f2mc16_reload_timer_device)

#endif
