// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    sh_adc.h

    SH Analog to Digital Converter subsystem


***************************************************************************/

#ifndef MAME_CPU_SH_SH_ADC_H
#define MAME_CPU_SH_SH_ADC_H

#pragma once

class sh7042_device;
class sh_intc_device;

class sh_adc_device : public device_t {
public:
	u16 addr_r(offs_t offset);
	u8 adcsr_r();
	u8 adcr_r();
	void adcsr_w(u8 data);
	void adcr_w(u8 data);
	void adtrg_w(int state);

	void set_suspend(bool suspend);
	u64 internal_update(u64 current_time);

protected:
	sh_adc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	required_device<sh7042_device> m_cpu;
	required_device<sh_intc_device> m_intc;
	int m_port_base, m_port_mask, m_port_shift;
	int m_intc_vector;
	bool m_is_hs;

	enum {
		T_SOFT  = 1<<0,
		T_TPU   = 1<<1,
		T_TIMER = 1<<2,
		T_EXT   = 1<<3
	};

	enum {
		F_ADF  = 0x80,
		F_ADIE = 0x40,
		F_ADST = 0x20
	};

	enum {
		IDLE = 0,
		ACTIVE = 1,
		HALTED = 2,
		REPEAT = 4,
		ROTATE = 8,
		DUAL = 16,
		BUFFER = 32,
		COUNTED = 64
	};

	u16 m_addr[8], m_buf[2];
	u8 m_adcsr, m_adcr;
	int m_register_mask;
	int m_trigger, m_start_mode, m_start_channel, m_end_channel, m_start_count;
	bool m_suspend_on_interrupt, m_analog_power_control;
	int m_mode, m_channel, m_count;
	bool m_analog_powered, m_adtrg;
	u64 m_next_event;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void sampling();
	void start_conversion();
	void conversion_wait(bool first, bool poweron, u64 current_time = 0);
	void buffer_value(int port, int buffer = 0);
	void commit_value(int reg, int buffer = 0);
	void timeout(u64 current_time);
	void done();

	int conversion_time(bool first, bool poweron);
	void mode_update();
	void do_buffering(int buffer);
	int get_channel_index(int count);
};

class sh_adc_ms_device : public sh_adc_device {
public:
	sh_adc_ms_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template<typename T, typename U> sh_adc_ms_device(const machine_config &mconfig, const char *tag, device_t *owner,
												   T &&cpu, U &&intc, int port_base, int vect) :
		sh_adc_ms_device(mconfig, tag, owner)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
		m_intc.set_tag(std::forward<U>(intc));
		m_is_hs = false;
		m_port_base = port_base;
		m_intc_vector = vect;
	}
};

class sh_adc_hs_device : public sh_adc_device {
public:
	sh_adc_hs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template<typename T, typename U> sh_adc_hs_device(const machine_config &mconfig, const char *tag, device_t *owner,
												   T &&cpu, U &&intc, int vect) :
		sh_adc_hs_device(mconfig, tag, owner)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
		m_intc.set_tag(std::forward<U>(intc));
		m_intc_vector = vect;
	}
};

DECLARE_DEVICE_TYPE(SH_ADC_MS, sh_adc_ms_device)
DECLARE_DEVICE_TYPE(SH_ADC_HS, sh_adc_hs_device)

#endif // MAME_CPU_SH_SH_ADC_H
