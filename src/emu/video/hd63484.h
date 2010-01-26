/*************************************************************************

  HD63484 ACRTC
  Advanced CRT Controller.

**************************************************************************/

#ifndef __HD63484_H__
#define __HD63484_H__

#include "devcb.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

#define HD63484_RAM_SIZE 0x100000

typedef struct _hd63484_interface hd63484_interface;
struct _hd63484_interface
{
	int        skattva_hack;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

DEVICE_GET_INFO( hd63484 );


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define HD63484 DEVICE_GET_INFO_NAME( hd63484 )

#define MDRV_HD63484_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, HD63484, 0) \
	MDRV_DEVICE_CONFIG(_interface)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

READ16_DEVICE_HANDLER( hd63484_status_r );
WRITE16_DEVICE_HANDLER( hd63484_address_w );
WRITE16_DEVICE_HANDLER( hd63484_data_w );
READ16_DEVICE_HANDLER( hd63484_data_r );

READ16_DEVICE_HANDLER( hd63484_ram_r );
READ16_DEVICE_HANDLER( hd63484_regs_r );
WRITE16_DEVICE_HANDLER( hd63484_ram_w );
WRITE16_DEVICE_HANDLER( hd63484_regs_w );

#endif /* __HD63484_H__ */


