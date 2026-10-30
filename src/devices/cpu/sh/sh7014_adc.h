// license:BSD-3-Clause
// copyright-holders:superctr
/***************************************************************************

  SH7016/SH7017 mid-speed A/D converter

***************************************************************************/

#ifndef MAME_CPU_SH_SH7014_ADC_H
#define MAME_CPU_SH_SH7014_ADC_H

#pragma once

#include "sh7014_intc.h"


class sh7014_adc_device : public device_t
{
public:
	sh7014_adc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T> sh7014_adc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&intc)
		: sh7014_adc_device(mconfig, tag, owner, clock)
	{
		m_intc.set_tag(std::forward<T>(intc));
	}

	template <int Channel> auto analog_callback() { return m_analog_cb[Channel].bind(); }

	void map(address_map &map) ATTR_COLD;

	uint16_t addr_r(offs_t offset);
	uint8_t adcsr_r();
	void adcsr_w(uint8_t data);
	uint8_t adcr_r();
	void adcr_w(uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum : uint8_t
	{
		ADCSR_ADF  = 0x80,
		ADCSR_ADIE = 0x40,
		ADCSR_ADST = 0x20,
		ADCSR_SCAN = 0x10,
		ADCSR_CKS  = 0x08,
		ADCSR_CH   = 0x03
	};

	TIMER_CALLBACK_MEMBER(conversion_done);

	void start_conversion();
	void update_interrupt();

	required_device<sh7014_intc_device> m_intc;
	devcb_read16::array<4> m_analog_cb;

	emu_timer *m_timer;

	uint16_t m_addr[4];
	uint8_t m_adcsr;
	uint8_t m_adcr;
	bool m_int_state;
};

DECLARE_DEVICE_TYPE(SH7014_ADC, sh7014_adc_device)

#endif // MAME_CPU_SH_SH7014_ADC_H
