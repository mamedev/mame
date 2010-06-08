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

DECLARE_LEGACY_DEVICE(ADC12130, adc12130);
DECLARE_LEGACY_DEVICE(ADC12132, adc12132);
DECLARE_LEGACY_DEVICE(ADC12138, adc12138);

#define MDRV_ADC12130_ADD(_tag, _config) \
	MDRV_DEVICE_ADD(_tag, ADC12130, 0) \
	MDRV_DEVICE_CONFIG(_config)

#define MDRV_ADC12132_ADD(_tag, _config) \
	MDRV_DEVICE_ADD(_tag, ADC12132, 0) \
	MDRV_DEVICE_CONFIG(_config)

#define MDRV_ADC12138_ADD(_tag, _config) \
	MDRV_DEVICE_ADD(_tag, ADC12138, 0) \
	MDRV_DEVICE_CONFIG(_config)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef double (*adc1213x_input_convert_func)(running_device *device, UINT8 input);

typedef struct _adc12138_interface adc12138_interface;
struct _adc12138_interface
{
	adc1213x_input_convert_func input_callback_r;
};

/***************************************************************************
    PROTOTYPES
***************************************************************************/

extern WRITE8_DEVICE_HANDLER( adc1213x_di_w );
extern WRITE8_DEVICE_HANDLER( adc1213x_cs_w );
extern WRITE8_DEVICE_HANDLER( adc1213x_sclk_w );
extern WRITE8_DEVICE_HANDLER( adc1213x_conv_w );
extern READ8_DEVICE_HANDLER( adc1213x_do_r );
extern READ8_DEVICE_HANDLER( adc1213x_eoc_r );

#endif	/* __ADC1213X_H__ */
