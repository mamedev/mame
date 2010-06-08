/***************************************************************************

    Motorola 6840 (PTM)

    Programmable Timer Module

***************************************************************************/

#ifndef __6840PTM_H__
#define __6840PTM_H__

#include "devlegcy.h"


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

DECLARE_LEGACY_DEVICE(PTM6840, ptm6840);

#define MDRV_PTM6840_ADD(_tag, _config) \
	MDRV_DEVICE_ADD(_tag, PTM6840, 0) \
	MDRV_DEVICE_CONFIG(_config)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


typedef struct _ptm6840_interface ptm6840_interface;
struct _ptm6840_interface
{
	double internal_clock;
	double external_clock[3];

	devcb_write8 out_func[3];	// function to call when output[idx] changes
	devcb_write_line irq_func;	// function called if IRQ line changes
};


/***************************************************************************
    PROTOTYPES
***************************************************************************/

int ptm6840_get_status( running_device *device, int clock );	// get whether timer is enabled
int ptm6840_get_irq( running_device *device );					// get IRQ state
UINT16 ptm6840_get_count( running_device *device, int counter );// get counter value
void ptm6840_set_ext_clock( running_device *device, int counter, double clock ); // set clock frequency
int ptm6840_get_ext_clock( running_device *device, int counter );// get clock frequency

WRITE8_DEVICE_HANDLER( ptm6840_set_g1 );	// set gate1 state
WRITE8_DEVICE_HANDLER( ptm6840_set_g2 );	// set gate2 state
WRITE8_DEVICE_HANDLER( ptm6840_set_g3 );	// set gate3 state
WRITE8_DEVICE_HANDLER( ptm6840_set_c1 );	// set clock1 state
WRITE8_DEVICE_HANDLER( ptm6840_set_c2 );	// set clock2 state
WRITE8_DEVICE_HANDLER( ptm6840_set_c3 );	// set clock3 state

WRITE8_DEVICE_HANDLER( ptm6840_write );
READ8_DEVICE_HANDLER( ptm6840_read );

#endif /* __6840PTM_H__ */
