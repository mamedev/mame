/***************************************************************************

    National Semiconductor ADC0831 / ADC0832 / ADC0834 / ADC0838

    8-Bit serial I/O A/D Converters with Muliplexer Options

***************************************************************************/

#ifndef __ADC083X_H__
#define __ADC083X_H__

#include "devcb.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* enumeration specifying which model we are emulating */
enum
{
	ADC0831,
	ADC0832,
	ADC0834,
	ADC0838,
	MAX_ADC083X_TYPES
};

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

#define ADC083X		DEVICE_GET_INFO_NAME(adc083x)

#define MDRV_ADC083X_ADD(_tag, _config) \
	MDRV_DEVICE_ADD(_tag, ADC083X, 0) \
	MDRV_DEVICE_CONFIG(_config)

#define MDRV_ADC083X_REMOVE(_tag) \
	MDRV_DEVICE_REMOVE(_tag)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef double (*adc083x_input_convert_func)(const device_config *device, UINT8 input);

typedef struct _adc083x_interface adc083x_interface;
struct _adc083x_interface
{
	int type;
	adc083x_input_convert_func input_callback_r;
};

/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* device interface */
DEVICE_GET_INFO( adc083x );

extern WRITE8_DEVICE_HANDLER( adc083x_cs_write );
extern WRITE8_DEVICE_HANDLER( adc083x_clk_write );
extern WRITE8_DEVICE_HANDLER( adc083x_di_write );
extern WRITE8_DEVICE_HANDLER( adc083x_se_write );
extern READ8_DEVICE_HANDLER( adc083x_sars_read );
extern READ8_DEVICE_HANDLER( adc083x_do_read );

#endif	/* __ADC083X_H__ */
