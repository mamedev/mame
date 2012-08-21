/*********************************************************************

    micropolis.h

    Implementations of the Micropolis
    floppy disk controller for the Sorcerer

*********************************************************************/

#ifndef __MICROPOLIS_H__
#define __MICROPOLIS_H__

#include "devcb.h"


/***************************************************************************
    MACROS
***************************************************************************/

DECLARE_LEGACY_DEVICE(MICROPOLIS, micropolis);



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* Interface */
typedef struct _micropolis_interface micropolis_interface;
struct _micropolis_interface
{
	devcb_read_line in_dden_func;
	devcb_write_line out_intrq_func;
	devcb_write_line out_drq_func;
	const char *floppy_drive_tags[4];
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
void micropolis_reset(device_t *device);

void micropolis_set_drive(device_t *device, UINT8); // set current drive (0-3)

READ8_DEVICE_HANDLER( micropolis_status_r );
READ8_DEVICE_HANDLER( micropolis_data_r );

WRITE8_DEVICE_HANDLER( micropolis_command_w );
WRITE8_DEVICE_HANDLER( micropolis_data_w );

READ8_DEVICE_HANDLER( micropolis_r );
WRITE8_DEVICE_HANDLER( micropolis_w );

extern const micropolis_interface default_micropolis_interface;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_MICROPOLIS_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, MICROPOLIS, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#endif /* __MICROPOLIS_H__ */
