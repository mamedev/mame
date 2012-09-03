/***************************************************************************

    National Semiconductor ADC0831 / ADC0832 / ADC0834 / ADC0838

    8-Bit serial I/O A/D Converters with Muliplexer Options

***************************************************************************/

#ifndef __ADC083X_H__
#define __ADC083X_H__

#include "devlegcy.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define ADC083X_CH0		0
#define ADC083X_CH1		1
#define ADC083X_CH2		2
#define ADC083X_CH3		3
#define ADC083X_CH4		4
#define ADC083X_CH5		5
#define ADC083X_CH6		6
#define ADC083X_CH7		7
#define ADC083X_COM		8
#define ADC083X_AGND	9
#define ADC083X_VREF	10

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class adc0831_device : public device_t
{
public:
	adc0831_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	adc0831_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~adc0831_device() { global_free(m_token); }

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

extern const device_type ADC0831;

class adc0832_device : public adc0831_device
{
public:
	adc0832_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type ADC0832;

class adc0834_device : public adc0831_device
{
public:
	adc0834_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type ADC0834;

class adc0838_device : public adc0831_device
{
public:
	adc0838_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type ADC0838;


#define MCFG_ADC0831_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, ADC0831, 0) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_ADC0832_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, ADC0832, 0) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_ADC0834_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, ADC0834, 0) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_ADC0838_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, ADC0838, 0) \
	MCFG_DEVICE_CONFIG(_config)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef double (*adc083x_input_convert_func)(device_t *device, UINT8 input);

typedef struct _adc083x_interface adc083x_interface;
struct _adc083x_interface
{
	adc083x_input_convert_func input_callback_r;
};


/***************************************************************************
    PROTOTYPES
***************************************************************************/

extern WRITE_LINE_DEVICE_HANDLER( adc083x_cs_write );
extern WRITE_LINE_DEVICE_HANDLER( adc083x_clk_write );
extern WRITE_LINE_DEVICE_HANDLER( adc083x_di_write );
extern WRITE_LINE_DEVICE_HANDLER( adc083x_se_write );
extern READ_LINE_DEVICE_HANDLER( adc083x_sars_read );
extern READ_LINE_DEVICE_HANDLER( adc083x_do_read );

#endif	/* __ADC083X_H__ */
