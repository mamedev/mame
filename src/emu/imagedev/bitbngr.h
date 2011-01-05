/*********************************************************************

    bitbngr.h

    TRS style "bitbanger" serial port

*********************************************************************/

#ifndef __BITBNGR_H__
#define __BITBNGR_H__

#include "image.h"


enum
{
	BITBANGER_PRINTER			= 0,
	BITBANGER_MODEM,
   BITBANGER_MODE_MAX,

	BITBANGER_150				= 0,
	BITBANGER_300,
	BITBANGER_600,
	BITBANGER_1200,
	BITBANGER_2400,
	BITBANGER_4800,
	BITBANGER_9600,
	BITBANGER_14400,
	BITBANGER_28800,
	BITBANGER_38400,
	BITBANGER_57600,
	BITBANGER_115200,
	BITBANGER_BAUD_MAX,

	BITBANGER_NEG40PERCENT  = 0,
	BITBANGER_NEG35PERCENT,
	BITBANGER_NEG30PERCENT,
	BITBANGER_NEG25PERCENT,
	BITBANGER_NEG20PERCENT,
	BITBANGER_NEG15PERCENT,
	BITBANGER_NEG10PERCENT,
	BITBANGER_NEG5PERCENT,
	BITBANGER_0PERCENT,
	BITBANGER_POS5PERCENT,
	BITBANGER_POS10PERCENT,
	BITBANGER_POS15PERCENT,
	BITBANGER_POS20PERCENT,
	BITBANGER_POS25PERCENT,
	BITBANGER_POS30PERCENT,
	BITBANGER_POS35PERCENT,
	BITBANGER_POS40PERCENT,
	BITBANGER_TUNE_MAX
};

/***************************************************************************
    CONSTANTS
***************************************************************************/
DECLARE_LEGACY_IMAGE_DEVICE(BITBANGER, bitbanger);

#define MCFG_BITBANGER_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, BITBANGER, 0) \
	MCFG_DEVICE_CONFIG(_intrf)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _bitbanger_config bitbanger_config;
struct _bitbanger_config
{
	/* callback to driver */
	void (*input_callback)(running_machine *machine, UINT8 bit);
	int default_mode;					   /* emulating a printer or modem */
	int default_baud;					   /* output bits per second */
	int default_tune;                /* fine tune adjustment to the baud */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* outputs data to a bitbanger port */
void bitbanger_output(device_t *device, int value);

/* ui functions */
const char *bitbanger_mode_string(device_t *device);
const char *bitbanger_baud_string(device_t *device);
const char *bitbanger_tune_string(device_t *device);
bool bitbanger_inc_mode(device_t *device, bool test);
bool bitbanger_dec_mode(device_t *device, bool test);
bool bitbanger_inc_tune(device_t *device, bool test);
bool bitbanger_dec_tune(device_t *device, bool test);
bool bitbanger_inc_baud(device_t *device, bool test);
bool bitbanger_dec_baud(device_t *device, bool test);

#endif /* __BITBNGR_H__ */
