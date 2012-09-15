/***************************************************************************

    National Semiconductor ADC1038

    10-Bit Serial I/O A/D Converters with Analog Multiplexer and
    Track/hold Function

***************************************************************************/

#ifndef __ADC1038_H__
#define __ADC1038_H__

#include "devlegcy.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef int (*adc1038_input_read_func)(device_t *device, int input);

struct adc1038_interface
{
	int gticlub_hack;
	adc1038_input_read_func input_callback_r;
};


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class adc1038_device : public device_t
{
public:
	adc1038_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~adc1038_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type ADC1038;


#define MCFG_ADC1038_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, ADC1038, 0) \
	MCFG_DEVICE_CONFIG(_config)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

extern READ_LINE_DEVICE_HANDLER( adc1038_do_read );
extern READ_LINE_DEVICE_HANDLER( adc1038_sars_read );
extern WRITE_LINE_DEVICE_HANDLER( adc1038_di_write );
extern WRITE_LINE_DEVICE_HANDLER( adc1038_clk_write );

#endif	/* __ADC1038_H__ */
