// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    Fujitsu Micro F2MC-16 series Programmable Pulse Generator

***************************************************************************/

#ifndef MAME_CPU_F2MC16_F2MC16_PPG_H
#define MAME_CPU_F2MC16_F2MC16_PPG_H

#pragma once

#include "f2mc16.h"
#include "f2mc16_intc.h"

class f2mc16_ppg_device :
	public device_t
{
public:
	f2mc16_ppg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, required_device<f2mc16_intc_device> &intc, uint8_t vector0, uint8_t vector1);
	f2mc16_ppg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template<unsigned N> auto output() { return m_output_cb[N].bind(); }

	void timebase_hz(uint32_t hz);

	uint8_t ppgc_r(offs_t offset);
	void ppgc_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	template<unsigned N> uint16_t prl_r();
	template<unsigned N> void prl_w(offs_t offset, uint16_t data, uint16_t mem_mask);

protected:
	// device_t
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void update();
	void update_pcnt();
	TIMER_CALLBACK_MEMBER(update_timebase_hz);
	TIMER_CALLBACK_MEMBER(update_peripheral_clock);
	template <unsigned N> TIMER_CALLBACK_MEMBER(timer_callback);

	f2mc16_device *m_cpu;
	required_device<f2mc16_intc_device> m_intc;
	uint8_t m_vector[2];
	devcb_write32::array<2> m_output_cb;
	uint32_t m_peripheral_clock_hz;
	attotime m_peripheral_clock_changed;
	uint32_t m_timebase_hz;
	attotime m_timebase_changed;

	emu_timer *m_timer[2];
	attotime m_start_time[2];
	attotime m_underflow_time[2];
	uint32_t m_clocksel[2];
	uint32_t m_hz[2];
	uint32_t m_duty[2];
	uint16_t m_pcnt[2][2];
	uint16_t m_pcntrl[2][2];
	uint8_t m_lh[2];
	uint8_t m_ppgc[2];
	uint8_t m_prll[2];
	uint8_t m_prlh[2];
};

DECLARE_DEVICE_TYPE(F2MC16_PPG, f2mc16_ppg_device)

#endif
