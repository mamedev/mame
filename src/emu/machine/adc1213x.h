/***************************************************************************

    National Semiconductor ADC12130 / ADC12132 / ADC12138

    Self-calibrating 12-bit Plus Sign Serial I/O A/D Converters with MUX
        and Sample/Hold

***************************************************************************/

#ifndef __ADC1213X_H__
#define __ADC1213X_H__

#include "devlegcy.h"


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class adc12138_device : public device_t
{
public:
	adc12138_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	adc12138_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~adc12138_device() { global_free(m_token); }

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


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef double (*adc1213x_input_convert_func)(device_t *device, UINT8 input);

struct adc12138_interface
{
	adc1213x_input_convert_func input_callback_r;
};

/***************************************************************************
    PROTOTYPES
***************************************************************************/

extern DECLARE_WRITE8_DEVICE_HANDLER( adc1213x_di_w );
extern DECLARE_WRITE8_DEVICE_HANDLER( adc1213x_cs_w );
extern DECLARE_WRITE8_DEVICE_HANDLER( adc1213x_sclk_w );
extern DECLARE_WRITE8_DEVICE_HANDLER( adc1213x_conv_w );
extern DECLARE_READ8_DEVICE_HANDLER( adc1213x_do_r );
extern DECLARE_READ8_DEVICE_HANDLER( adc1213x_eoc_r );

#endif	/* __ADC1213X_H__ */
