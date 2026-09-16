// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    Fujitsu Micro F2MC-16 series I/O Port

***************************************************************************/

#ifndef MAME_CPU_F2MC16_F2MC16_ADC_H
#define MAME_CPU_F2MC16_F2MC16_ADC_H

#include "f2mc16_intc.h"

#pragma once

class f2mc16_adc_device :
	public device_t
{
public:
	f2mc16_adc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, required_device<f2mc16_intc_device> &intc, uint8_t vector);
	f2mc16_adc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template<unsigned N> auto channel() { return m_channel_cb[N].bind(); }

	void i2osclr(int state);

	void internal_timer_hz(uint32_t hz);
	void atgx(int state);

	uint16_t adcs_r();
	void adcs_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t adcr_r();
	void adcr_w(offs_t offset, uint16_t data, uint16_t mem_mask);

protected:
	// device_t
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_peripheral_clock);
	TIMER_CALLBACK_MEMBER(update_internal_timer);
	TIMER_CALLBACK_MEMBER(timer_callback);

	void trigger();
	void update();
	void start_conversion();
	void update_conversion_ticks();

	required_device<f2mc16_intc_device> m_intc;
	devcb_read16::array<16> m_channel_cb;
	uint8_t m_vector;

	emu_timer *m_timer;
	attotime m_peripheral_clock_changed;
	attotime m_internal_timer_changed;
	attotime m_internal_timer_expired;
	attotime m_conversion_start_time;
	attotime m_conversion_finished;
	uint32_t m_peripheral_clock_hz;
	uint32_t m_internal_timer_hz;
	uint8_t m_conversion_ticks;
	int8_t m_channel;
	int8_t m_atgx;
	int8_t m_i2osclr;
	uint16_t m_adcs;
	uint16_t m_adcr;
};

DECLARE_DEVICE_TYPE(F2MC16_ADC, f2mc16_adc_device)

#endif
