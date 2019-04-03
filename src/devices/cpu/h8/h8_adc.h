// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_adc.h

    H8 Analog to Digital Converter subsystem


***************************************************************************/

#ifndef MAME_CPU_H8_H8_ADC_H
#define MAME_CPU_H8_H8_ADC_H

#pragma once

#include "h8.h"
#include "h8_intc.h"

class h8_adc_device : public device_t {
public:
	void set_info(const char *intc_tag, int vect);

	DECLARE_READ8_MEMBER(addr8_r);
	DECLARE_READ16_MEMBER(addr16_r);
	DECLARE_READ8_MEMBER(adcsr_r);
	DECLARE_READ8_MEMBER(adcr_r);
	DECLARE_WRITE8_MEMBER(adcsr_w);
	DECLARE_WRITE8_MEMBER(adcr_w);
	DECLARE_WRITE_LINE_MEMBER(adtrg_w);

	void set_suspend(bool suspend);
	uint64_t internal_update(uint64_t current_time);

protected:
	h8_adc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	required_device<h8_device> cpu;
	h8_intc_device *intc;
	address_space *io;
	const char *intc_tag;
	int intc_vector;

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

	uint16_t addr[8], buf[2];
	uint8_t adcsr, adcr;
	int register_mask;
	int trigger, start_mode, start_channel, end_channel, start_count;
	bool suspend_on_interrupt, analog_power_control;
	int mode, channel, count;
	bool analog_powered, adtrg;
	uint64_t next_event;

	virtual void device_start() override;
	virtual void device_reset() override;

	void sampling();
	void start_conversion();
	void conversion_wait(bool first, bool poweron, uint64_t current_time = 0);
	void buffer_value(int port, int buffer = 0);
	void commit_value(int reg, int buffer = 0);
	void timeout(uint64_t current_time);
	void done();

	virtual int conversion_time(bool first, bool poweron) = 0;
	virtual void mode_update() = 0;
	virtual void do_buffering(int buffer);
	virtual int get_channel_index(int count);
};

class h8_adc_3337_device : public h8_adc_device {
public:
	h8_adc_3337_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	h8_adc_3337_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *intc_tag, int vect)
		: h8_adc_3337_device(mconfig, tag, owner, 0)
	{
		set_info(intc_tag, vect);
	}

protected:
	virtual int conversion_time(bool first, bool poweron) override;
	virtual void mode_update() override;
};

class h8_adc_3006_device : public h8_adc_device {
public:
	h8_adc_3006_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	h8_adc_3006_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *intc_tag, int vect)
		: h8_adc_3006_device(mconfig, tag, owner, 0)
	{
		set_info(intc_tag, vect);
	}

protected:
	virtual int conversion_time(bool first, bool poweron) override;
	virtual void mode_update() override;
};

class h8_adc_2245_device : public h8_adc_device {
public:
	h8_adc_2245_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	h8_adc_2245_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *intc_tag, int vect)
		: h8_adc_2245_device(mconfig, tag, owner, 0)
	{
		set_info(intc_tag, vect);
	}

protected:
	virtual int conversion_time(bool first, bool poweron) override;
	virtual void mode_update() override;
};

class h8_adc_2320_device : public h8_adc_device {
public:
	h8_adc_2320_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	h8_adc_2320_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *intc_tag, int vect)
		: h8_adc_2320_device(mconfig, tag, owner, 0)
	{
		set_info(intc_tag, vect);
	}

protected:
	virtual int conversion_time(bool first, bool poweron) override;
	virtual void mode_update() override;
};

class h8_adc_2357_device : public h8_adc_device {
public:
	h8_adc_2357_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	h8_adc_2357_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *intc_tag, int vect)
		: h8_adc_2357_device(mconfig, tag, owner, 0)
	{
		set_info(intc_tag, vect);
	}

protected:
	virtual int conversion_time(bool first, bool poweron) override;
	virtual void mode_update() override;
};

class h8_adc_2655_device : public h8_adc_device {
public:
	h8_adc_2655_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	h8_adc_2655_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *intc_tag, int vect)
		: h8_adc_2655_device(mconfig, tag, owner, 0)
	{
		set_info(intc_tag, vect);
	}

protected:
	virtual int conversion_time(bool first, bool poweron) override;
	virtual void mode_update() override;
	virtual void do_buffering(int buffer) override;
	virtual int get_channel_index(int count) override;
};

DECLARE_DEVICE_TYPE(H8_ADC_3337, h8_adc_3337_device)
DECLARE_DEVICE_TYPE(H8_ADC_3006, h8_adc_3006_device)
DECLARE_DEVICE_TYPE(H8_ADC_2245, h8_adc_2245_device)
DECLARE_DEVICE_TYPE(H8_ADC_2320, h8_adc_2320_device)
DECLARE_DEVICE_TYPE(H8_ADC_2357, h8_adc_2357_device)
DECLARE_DEVICE_TYPE(H8_ADC_2655, h8_adc_2655_device)

#endif // MAME_CPU_H8_H8_ADC_H
