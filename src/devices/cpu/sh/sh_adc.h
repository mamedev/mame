// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    sh_adc.h

    SH Analog to Digital Converter subsystem

***************************************************************************/

#ifndef MAME_CPU_SH_SH_ADC_H
#define MAME_CPU_SH_SH_ADC_H

#pragma once

// To generalize eventually
class sh7042_device;

class sh_adc_device : public device_t {
public:
	sh_adc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename T> sh_adc_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu)
		: sh_adc_device(mconfig, tag, owner)
	{
		set_info(cpu);
	}

	template<typename T> void set_info(T &&cpu) { m_cpu.set_tag(std::forward<T>(cpu)); }

	u16 addr_r(offs_t offset);
	u8 adcsr_r();
	u8 adcr_r();
	void adcsr_w(u8 data);
	void adcr_w(u8 data);

protected:
	enum {
		CSR_ADF  = 0x80,
		CSR_ADIE = 0x40,
		CSR_ADST = 0x20,
		CSR_CKS  = 0x10,
		CSR_GRP  = 0x08,
		CSR_CHAN = 0x07
	};

	enum {
		CR_PWR   = 0x40,
		CR_TRGS  = 0x30,
		CR_SCAN  = 0x08,
		CR_SIM   = 0x04,
		CR_BUF   = 0x03
	};
		
	required_device<sh7042_device> m_cpu;

	uint16_t m_addr[8];
	uint8_t m_adcsr, m_adcr;


	virtual void device_start() override;
	virtual void device_reset() override;
};

DECLARE_DEVICE_TYPE(SH_ADC, sh_adc_device)

#endif
