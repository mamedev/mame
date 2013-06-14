/***************************************************************************

    National Semiconductor ADC1038

    10-Bit Serial I/O A/D Converters with Analog Multiplexer and
    Track/hold Function

***************************************************************************/

#ifndef __ADC1038_H__
#define __ADC1038_H__



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef int (*adc1038_input_read_func)(device_t *device, int input);

struct adc1038_interface
{
	int m_gticlub_hack;
	adc1038_input_read_func input_callback_r;
};


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class adc1038_device : public device_t,
										public adc1038_interface
{
public:
	adc1038_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~adc1038_device() {}

	DECLARE_READ_LINE_MEMBER( do_read );
	DECLARE_READ_LINE_MEMBER( sars_read );
	DECLARE_WRITE_LINE_MEMBER( di_write );
	DECLARE_WRITE_LINE_MEMBER( clk_write );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	adc1038_input_read_func           m_input_callback_r_func;

	private:
	// internal state
	int m_cycle;
	int m_clk;
	int m_adr;
	int m_data_in;
	int m_data_out;
	int m_adc_data;
	int m_sars;
};

extern const device_type ADC1038;


#define MCFG_ADC1038_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, ADC1038, 0) \
	MCFG_DEVICE_CONFIG(_config)


#endif  /* __ADC1038_H__ */
