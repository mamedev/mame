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

typedef double (*adc1213x_input_convert_func)(device_t *device, UINT8 input);

struct adc12138_interface
{
	adc1213x_input_convert_func input_callback_r;
};

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class adc12138_device : public device_t,
										public adc12138_interface
{
public:
	adc12138_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	adc12138_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	~adc12138_device() {}

	DECLARE_WRITE8_MEMBER( di_w );
	DECLARE_WRITE8_MEMBER( cs_w );
	DECLARE_WRITE8_MEMBER( sclk_w );
	DECLARE_WRITE8_MEMBER( conv_w );
	DECLARE_READ8_MEMBER( do_r );
	DECLARE_READ8_MEMBER( eoc_r );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	void convert(int channel, int bits16, int lsbfirst);

	adc1213x_input_convert_func m_input_callback_r_func;

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

#define MCFG_ADC12130_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, ADC12130, 0) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_ADC12132_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, ADC12132, 0) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_ADC12138_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, ADC12138, 0) \
	MCFG_DEVICE_CONFIG(_config)


#endif  /* __ADC1213X_H__ */
