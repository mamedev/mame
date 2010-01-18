#ifndef NAMCO53_H
#define NAMCO53_H

#include "devcb.h"


typedef struct _namco_53xx_interface namco_53xx_interface;
struct _namco_53xx_interface
{
	devcb_read8		k;			/* read handlers for K port */
	devcb_read8 	in[4];		/* read handlers for ports A-D */
	devcb_write8	p;			/* write handler for P port */
};


#define MDRV_NAMCO_53XX_ADD(_tag, _clock, _interface) \
	MDRV_DEVICE_ADD(_tag, NAMCO_53XX, _clock) \
	MDRV_DEVICE_CONFIG(_interface)


void namco_53xx_read_request(running_device *device);
READ8_DEVICE_HANDLER( namco_53xx_read );


/* device get info callback */
#define NAMCO_53XX DEVICE_GET_INFO_NAME(namco_53xx)
DEVICE_GET_INFO( namco_53xx );


#endif	/* NAMCO53_H */
