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

typedef int (*adc1038_input_read_func)(running_device *device, int input);

typedef struct _adc1038_interface adc1038_interface;
struct _adc1038_interface
{
	int gticlub_hack;
	adc1038_input_read_func input_callback_r;
};


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

DECLARE_LEGACY_DEVICE(ADC1038, adc1038);

#define MDRV_ADC1038_ADD(_tag, _config) \
	MDRV_DEVICE_ADD(_tag, ADC1038, 0) \
	MDRV_DEVICE_CONFIG(_config)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

extern READ_LINE_DEVICE_HANDLER( adc1038_do_read );
extern READ_LINE_DEVICE_HANDLER( adc1038_sars_read );
extern WRITE_LINE_DEVICE_HANDLER( adc1038_di_write );
extern WRITE_LINE_DEVICE_HANDLER( adc1038_clk_write );

#endif	/* __ADC1038_H__ */
