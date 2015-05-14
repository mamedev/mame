// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    National Semiconductor ADC12130 / ADC12132 / ADC12138

    Self-calibrating 12-bit Plus Sign Serial I/O A/D Converters with MUX
        and Sample/Hold

***************************************************************************/

#ifndef __ADC1213X_H__
#define __ADC1213X_H__


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef device_delegate<double (UINT8 input)> adc1213x_ipt_convert_delegate;
#define ADC12138_IPT_CONVERT_CB(name)  double name(UINT8 input)

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class adc12138_device : public device_t
{
public:
	adc12138_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	adc12138_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	~adc12138_device() {}

	static void set_ipt_convert_callback(device_t &device, adc1213x_ipt_convert_delegate callback) { downcast<adc12138_device &>(device).m_ipt_read_cb = callback; }

	DECLARE_WRITE8_MEMBER( di_w );
	DECLARE_WRITE8_MEMBER( cs_w );
	DECLARE_WRITE8_MEMBER( sclk_w );
	DECLARE_WRITE8_MEMBER( conv_w );
	DECLARE_READ8_MEMBER( do_r );
	DECLARE_READ8_MEMBER( eoc_r );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	void convert(int channel, int bits16, int lsbfirst);

	adc1213x_ipt_convert_delegate m_ipt_read_cb;

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

extern const device_type ADC12138;

class adc12130_device : public adc12138_device
{
public:
	adc12130_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type ADC12130;

class adc12132_device : public adc12138_device
{
public:
	adc12132_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type ADC12132;


#define MCFG_ADC1213X_IPT_CONVERT_CB(_class, _method) \
	adc12138_device::set_ipt_convert_callback(*device, adc1213x_ipt_convert_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#endif  /* __ADC1213X_H__ */
