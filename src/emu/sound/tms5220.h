#pragma once

#ifndef __TMS5220_H__
#define __TMS5220_H__

#include "devcb.h"

/* clock rate = 80 * output sample rate,     */
/* usually 640000 for 8000 Hz sample rate or */
/* usually 800000 for 10000 Hz sample rate.  */

typedef struct _tms5220_interface tms5220_interface;
struct _tms5220_interface
{
	devcb_write_line irq_func;		/* IRQ callback function */

	int (*read)(const device_config *device, int count);			/* speech ROM read callback */
	void (*load_address)(const device_config *device, int data);	/* speech ROM load address callback */
	void (*read_and_branch)(const device_config *device);		/* speech ROM read and branch callback */
};

/* Control lines - once written to will switch interface into
 * "true" timing behaviour.
 */

/* all lines with suffix q are active low! */

WRITE_LINE_DEVICE_HANDLER( tms5220_rsq_w );
WRITE_LINE_DEVICE_HANDLER( tms5220_wsq_w );

WRITE8_DEVICE_HANDLER( tms5220_data_w );
READ8_DEVICE_HANDLER( tms5220_status_r );

READ_LINE_DEVICE_HANDLER( tms5220_readyq_r );
READ_LINE_DEVICE_HANDLER( tms5220_intq_r );


double tms5220_time_to_ready(const device_config *device);

void tms5220_set_frequency(const device_config *device, int frequency);

DEVICE_GET_INFO( tms5220c );
DEVICE_GET_INFO( tms5220 );
DEVICE_GET_INFO( tmc0285 );
DEVICE_GET_INFO( tms5200 );

#define SOUND_TMS5220C DEVICE_GET_INFO_NAME( tms5220c )
#define SOUND_TMS5220 DEVICE_GET_INFO_NAME( tms5220 )
#define SOUND_TMC0285 DEVICE_GET_INFO_NAME( tmc0285 )
#define SOUND_TMS5200 DEVICE_GET_INFO_NAME( tms5200 )


#endif /* __TMS5220_H__ */
