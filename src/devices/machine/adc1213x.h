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
    TYPE DEFINITIONS
***************************************************************************/

#define ADC12138_IPT_CONVERT_CB(name)  double name(uint8_t input)

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class adc12138_device : public device_t
{
public:
	typedef device_delegate<double (uint8_t input)> ipt_convert_delegate;

	adc12138_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename Object> void set_ipt_convert_callback(Object &&cb) { m_ipt_read_cb = std::forward<Object>(cb); }

	DECLARE_WRITE8_MEMBER( di_w );
	DECLARE_WRITE8_MEMBER( cs_w );
	DECLARE_WRITE8_MEMBER( sclk_w );
	DECLARE_WRITE8_MEMBER( conv_w );
	DECLARE_READ8_MEMBER( do_r );
	DECLARE_READ8_MEMBER( eoc_r );

protected:
	adc12138_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

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


#define MCFG_ADC1213X_IPT_CONVERT_CB(_class, _method) \
	downcast<adc12138_device &>(*device).set_ipt_convert_callback(adc12138_device::ipt_convert_delegate(&_class::_method, #_class "::" #_method, this));

#endif // MAME_MACHINE_ADC1213X_H
