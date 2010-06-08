#ifndef NAMCO51_H
#define NAMCO51_H

#include "devlegcy.h"


typedef struct _namco_51xx_interface namco_51xx_interface;
struct _namco_51xx_interface
{
	devcb_read8 	in[4];		/* read handlers for ports A-D */
	devcb_write8	out[2];		/* write handlers for ports A-B */
};


#define MDRV_NAMCO_51XX_ADD(_tag, _clock, _interface) \
	MDRV_DEVICE_ADD(_tag, NAMCO_51XX, _clock) \
	MDRV_DEVICE_CONFIG(_interface)


READ8_DEVICE_HANDLER( namco_51xx_read );
WRITE8_DEVICE_HANDLER( namco_51xx_write );


DECLARE_LEGACY_DEVICE(NAMCO_51XX, namco_51xx);


#endif	/* NAMCO51_H */
