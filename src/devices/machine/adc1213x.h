// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    National Semiconductor ADC12130 / ADC12132 / ADC12138

    Self-calibrating 12-bit Plus Sign Serial I/O A/D Converters with MUX
        and Sample/Hold

***************************************************************************/

#ifndef MAME_MACHINE_ADC1213X_H
#define MAME_MACHINE_ADC1213X_H

#pragma once


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class adc12138_device : public device_t
{
public:
	typedef device_delegate<double (uint8_t input)> ipt_convert_delegate;

	adc12138_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename... T> void set_ipt_convert_callback(T &&... args) { m_ipt_read_cb.set(std::forward<T>(args)...); }

	void di_w(u8 data);
	void cs_w(u8 data);
	void sclk_w(u8 data);
	void conv_w(u8 data = 0);
	u8 do_r();
	u8 eoc_r();

protected:
	adc12138_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void convert(int channel, int bits16, int lsbfirst);

	ipt_convert_delegate m_ipt_read_cb;

private:
	// internal state
	int m_cycle;
	int m_data_out;
	int m_data_in;
	int m_conv_mode;
	int m_auto_cal;
	int m_auto_zero;
	int m_acq_time;
	int m_data_out_sign;
	int m_input_shift_reg;
	int m_output_shift_reg;
	int m_end_conv;
};


class adc12130_device : public adc12138_device
{
public:
	adc12130_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class adc12132_device : public adc12138_device
{
public:
	adc12132_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(ADC12138, adc12138_device)
DECLARE_DEVICE_TYPE(ADC12130, adc12130_device)
DECLARE_DEVICE_TYPE(ADC12132, adc12132_device)

#endif // MAME_MACHINE_ADC1213X_H
