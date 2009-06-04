/***************************************************************************

    National Semiconductor ADC12130 / ADC12132 / ADC12138

    Self-calibrating 12-bit Plus Sign Serial I/O A/D Converters with MUX
        and Sample/Hold

***************************************************************************/

#ifndef __ADC1213X_H__
#define __ADC1213X_H__

#include "devcb.h"

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define ADC12130		DEVICE_GET_INFO_NAME(adc12130)
#define ADC12132		DEVICE_GET_INFO_NAME(adc12132)
#define ADC12138		DEVICE_GET_INFO_NAME(adc12138)

#define MDRV_ADC12130_ADD(_tag, _config) \
	MDRV_DEVICE_ADD(_tag, ADC12130, 0) \
	MDRV_DEVICE_CONFIG(_config)

#define MDRV_ADC12130_REMOVE(_tag) \
	MDRV_DEVICE_REMOVE(_tag)

#define MDRV_ADC12132_ADD(_tag, _config) \
	MDRV_DEVICE_ADD(_tag, ADC12132, 0) \
	MDRV_DEVICE_CONFIG(_config)

#define MDRV_ADC12132_REMOVE(_tag) \
	MDRV_DEVICE_REMOVE(_tag)

#define MDRV_ADC12138_ADD(_tag, _config) \
	MDRV_DEVICE_ADD(_tag, ADC12138, 0) \
	MDRV_DEVICE_CONFIG(_config)

#define MDRV_ADC12138_REMOVE(_tag) \
	MDRV_DEVICE_REMOVE(_tag)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef double (*adc1213x_input_convert_func)(const device_config *device, UINT8 input);

typedef struct _adc12138_interface adc12138_interface;
struct _adc12138_interface
{
	adc1213x_input_convert_func input_callback_r;
};

/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* device interface */
DEVICE_GET_INFO( adc12130 );
DEVICE_GET_INFO( adc12132 );
DEVICE_GET_INFO( adc12138 );

extern WRITE8_DEVICE_HANDLER( adc1213x_di_w );
extern WRITE8_DEVICE_HANDLER( adc1213x_cs_w );
extern WRITE8_DEVICE_HANDLER( adc1213x_sclk_w );
extern WRITE8_DEVICE_HANDLER( adc1213x_conv_w );
extern READ8_DEVICE_HANDLER( adc1213x_do_r );
extern READ8_DEVICE_HANDLER( adc1213x_eoc_r );

#endif	/* __ADC1213X_H__ */
