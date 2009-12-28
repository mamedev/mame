/***************************************************************************

    z80daisy.h

    Z80/180 daisy chaining support functions.

***************************************************************************/

#pragma once

#ifndef __Z80DAISY_H__
#define __Z80DAISY_H__

#include "devintrf.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* these constants are returned from the irq_state function */
#define Z80_DAISY_INT	0x01		/* interrupt request mask */
#define Z80_DAISY_IEO	0x02		/* interrupt disable mask (IEO) */


enum
{
	DEVINFO_FCT_IRQ_STATE = DEVINFO_FCT_DEVICE_SPECIFIC,	/* R/O: z80_daisy_irq_state */
	DEVINFO_FCT_IRQ_ACK,									/* R/O: z80_daisy_irq_ack */
	DEVINFO_FCT_IRQ_RETI									/* R/O: z80_daisy_irq_reti */
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* per-device callback functions */
typedef int (*z80_daisy_irq_state)(const device_config *device);
typedef int (*z80_daisy_irq_ack)(const device_config *device);
typedef int (*z80_daisy_irq_reti)(const device_config *device);


/* opaque internal daisy chain state */
typedef struct _z80_daisy_state z80_daisy_state;


/* daisy chain structure */
typedef struct _z80_daisy_chain z80_daisy_chain;
struct _z80_daisy_chain
{
	const char *	devname;					/* name of the device */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

z80_daisy_state *z80daisy_init(const device_config *cpudevice, const z80_daisy_chain *daisy);

void z80daisy_reset(z80_daisy_state *daisy);
int z80daisy_update_irq_state(z80_daisy_state *chain);
int z80daisy_call_ack_device(z80_daisy_state *chain);
void z80daisy_call_reti_device(z80_daisy_state *chain);

#endif
