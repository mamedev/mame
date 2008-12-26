#pragma once

#ifndef __5110INTF_H__
#define __5110INTF_H__

/* clock rate = 80 * output sample rate,     */
/* usually 640000 for 8000 Hz sample rate or */
/* usually 800000 for 10000 Hz sample rate.  */

typedef struct _tms5110_interface tms5110_interface;
struct _tms5110_interface
{
	int (*M0_callback)(const device_config *device);	/* function to be called when chip requests another bit */
	void (*load_address)(int addr);	/* speech ROM load address callback */
};

WRITE8_HANDLER( tms5110_ctl_w );
WRITE8_HANDLER( tms5110_pdc_w );

READ8_HANDLER( tms5110_status_r );
int tms5110_ready_r(void);

void tms5110_set_frequency(int frequency);

SND_GET_INFO( tms5110 );
SND_GET_INFO( tms5100 );
SND_GET_INFO( tms5110a );
SND_GET_INFO( cd2801 );
SND_GET_INFO( tmc0281 );
SND_GET_INFO( cd2802 );
SND_GET_INFO( m58817 );

#endif /* __5110INTF_H__ */
