#pragma once

#ifndef __TMS5220_H__
#define __TMS5220_H__

#include "devlegcy.h"


/* clock rate = 80 * output sample rate,     */
/* usually 640000 for 8000 Hz sample rate or */
/* usually 800000 for 10000 Hz sample rate.  */

typedef struct _tms5220_interface tms5220_interface;
struct _tms5220_interface
{
	devcb_write_line irq_func;		/* IRQ callback function, active low, i.e. state=0 */
	devcb_write_line readyq_func;	/* Ready callback function, active low, i.e. state=0 */

	int (*read)(device_t *device, int count);			/* speech ROM read callback */
	void (*load_address)(device_t *device, int data);	/* speech ROM load address callback */
	void (*read_and_branch)(device_t *device);		/* speech ROM read and branch callback */
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


double tms5220_time_to_ready(device_t *device);

void tms5220_set_frequency(device_t *device, int frequency);

DECLARE_LEGACY_SOUND_DEVICE(TMS5220C, tms5220c);
DECLARE_LEGACY_SOUND_DEVICE(TMS5220, tms5220);
DECLARE_LEGACY_SOUND_DEVICE(TMC0285, tmc0285);
DECLARE_LEGACY_SOUND_DEVICE(TMS5200, tms5200);


#endif /* __TMS5220_H__ */
