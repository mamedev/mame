#ifndef NAMCO53_H
#define NAMCO53_H

#include "devlegcy.h"


typedef struct _namco_53xx_interface namco_53xx_interface;
struct _namco_53xx_interface
{
	devcb_read8		k;			/* read handlers for K port */
	devcb_read8 	in[4];		/* read handlers for ports A-D */
	devcb_write8	p;			/* write handler for P port */
};


#define MCFG_NAMCO_53XX_ADD(_tag, _clock, _interface) \
	MCFG_DEVICE_ADD(_tag, NAMCO_53XX, _clock) \
	MCFG_DEVICE_CONFIG(_interface)


void namco_53xx_read_request(device_t *device);
READ8_DEVICE_HANDLER( namco_53xx_read );


DECLARE_LEGACY_DEVICE(NAMCO_53XX, namco_53xx);


#endif	/* NAMCO53_H */
