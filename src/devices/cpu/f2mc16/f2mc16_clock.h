// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    Fujitsu Micro F2MC-16 series Clock Generator

***************************************************************************/

#ifndef MAME_CPU_F2MC16_F2MC16_CLOCK_H
#define MAME_CPU_F2MC16_F2MC16_CLOCK_H

#pragma once

#include "f2mc16.h"
#include "f2mc16_intc.h"

class f2mc16_clock_generator_device :
	public device_t
{
public:
	f2mc16_clock_generator_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, required_device<f2mc16_intc_device> &intc, uint8_t m_tbtc_vector);
	f2mc16_clock_generator_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto timebase_hz() { return m_timebase_hz_cb.bind(); }

	uint8_t lpmcr_r();
	void lpmcr_w(uint8_t data);

	uint8_t wdtc_r();
	void wdtc_w(uint8_t data);

	uint8_t tbtc_r();
	void tbtc_w(uint8_t data);
	uint8_t ckscr_r();
	void ckscr_w(uint8_t data);

protected:
	// device_t
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;
	virtual void device_reset() override ATTR_COLD;

	void timebase_reset();
	void timebase_update();
	void timebase_update_counter();
	TIMER_CALLBACK_MEMBER(soft_reset);
	TIMER_CALLBACK_MEMBER(update_mainclock_hz);
	TIMER_CALLBACK_MEMBER(timebase_timer_callback);

	f2mc16_device *m_cpu;
	required_device<f2mc16_intc_device> m_intc;
	uint8_t m_tbtc_vector;
	devcb_write32 m_timebase_hz_cb;

	emu_timer *m_timebase_timer;
	attotime m_timebase_start_time;
	attotime m_tbtc_overflow_time;
	attotime m_watchdog_overflow_time;
	attotime m_mainclock_changed;
	uint32_t m_timebase_counter;
	uint32_t m_tbtc_hz;
	uint32_t m_mainclock_hz;
	uint8_t m_reset_reason;
	uint8_t m_watchdog_overflow_count;
	uint8_t m_wt;
	uint8_t m_ckscr;
	uint8_t m_lpmcr;
	uint8_t m_tbtc;
	uint8_t m_wdtc;
};

DECLARE_DEVICE_TYPE(F2MC16_CLOCK_GENERATOR, f2mc16_clock_generator_device)

#endif
