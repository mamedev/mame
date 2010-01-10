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

typedef int (*adc1038_input_read_func)(const device_config *device, int input);

typedef struct _adc1038_interface adc1038_interface;
struct _adc1038_interface
{
	int gticlub_hack;
	adc1038_input_read_func input_callback_r;
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

DEVICE_GET_INFO( adc1038 );


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define ADC1038		DEVICE_GET_INFO_NAME( adc1038 )

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
