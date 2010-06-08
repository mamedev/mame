/*************************************************************************

    SMC91C9X ethernet controller implementation

    by Aaron Giles

**************************************************************************/

#ifndef __SMC91C9X__
#define __SMC91C9X__

#include "devlegcy.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*smc91c9x_irq_func)(running_device *device, int state);


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

DECLARE_LEGACY_DEVICE(SMC91C94, smc91c94);
DECLARE_LEGACY_DEVICE(SMC91C96, smc91c96);

#endif
