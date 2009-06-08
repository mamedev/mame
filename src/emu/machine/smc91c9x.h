/*************************************************************************

    SMC91C9X ethernet controller implementation

    by Aaron Giles

**************************************************************************/

#ifndef __SMC91C9X__
#define __SMC91C9X__


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*smc91c9x_irq_func)(const device_config *device, int state);


typedef struct _smc91c9x_config smc91c9x_config;
struct _smc91c9x_config
{
	smc91c9x_irq_func	interrupt;
};



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_SMC91C94_ADD(_tag, _callback) \
	MDRV_DEVICE_ADD(_tag, SMC91C94, 0) \
	MDRV_DEVICE_CONFIG_DATAPTR(smc91c9x_config, interrupt, _callback)

#define MDRV_SMC91C96_ADD(_tag, _callback) \
	MDRV_DEVICE_ADD(_tag, SMC91C96, 0) \
	MDRV_DEVICE_CONFIG_DATAPTR(smc91c9x_config, interrupt, _callback)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

READ16_DEVICE_HANDLER( smc91c9x_r );
WRITE16_DEVICE_HANDLER( smc91c9x_w );


/* ----- device interface ----- */

/* device get info callbacks */
#define SMC91C94 DEVICE_GET_INFO_NAME(smc91c94)
#define SMC91C96 DEVICE_GET_INFO_NAME(smc91c96)
DEVICE_GET_INFO( smc91c94 );
DEVICE_GET_INFO( smc91c96 );

#endif
