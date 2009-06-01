/***************************************************************************

    National Semiconductor ADC12130 / ADC12132 / ADC12138

    Self-calibrating 12-bit Plus Sign Serial I/O A/D Converters with MUX 
        and Sample/Hold

***************************************************************************/

#ifndef __ADC1213X_H__
#define __ADC1213X_H__

#include "devcb.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* enumeration specifying which model we are emulating */
enum
{
	ADC12130,
	ADC12132,
	ADC12138,
	MAX_ADC1213X_TYPES
};

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define ADC1213X		DEVICE_GET_INFO_NAME(adc1213x)

#define MDRV_ADC1213X_ADD(_tag, _config) \
	MDRV_DEVICE_ADD(_tag, ADC1213X, 0) \
	MDRV_DEVICE_CONFIG(_config)

#define MDRV_ADC1213X_REMOVE(_tag) \
	MDRV_DEVICE_REMOVE(_tag)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef double (*adc1213x_input_convert_func)(const device_config *device, UINT8 input);

typedef struct _adc1213x_interface adc1213x_interface;
struct _adc1213x_interface
{
	int type;
	adc1213x_input_convert_func input_callback_r;
};

/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* device interface */
DEVICE_GET_INFO( adc1213x );

extern WRITE8_DEVICE_HANDLER( adc1213x_di_w );
extern WRITE8_DEVICE_HANDLER( adc1213x_cs_w );
extern WRITE8_DEVICE_HANDLER( adc1213x_sclk_w );
extern WRITE8_DEVICE_HANDLER( adc1213x_conv_w );
extern READ8_DEVICE_HANDLER( adc1213x_do_r );
extern READ8_DEVICE_HANDLER( adc1213x_eoc_r );

#endif	/* __ADC1213X_H__ */
