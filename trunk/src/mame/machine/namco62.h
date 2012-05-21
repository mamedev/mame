#ifndef NAMCO62_H
#define NAMCO62_H

#include "devlegcy.h"


typedef struct _namco_62xx_interface namco_62xx_interface;
struct _namco_62xx_interface
{
	devcb_read8 	in[4];		/* read handlers for ports A-D */
	devcb_write8	out[2];		/* write handlers for ports A-B */
};


#define MCFG_NAMCO_62XX_ADD(_tag, _clock, _interface) \
	MCFG_DEVICE_ADD(_tag, NAMCO_62XX, _clock) \
	MCFG_DEVICE_CONFIG(_interface)


READ8_DEVICE_HANDLER( namco_62xx_read );
WRITE8_DEVICE_HANDLER( namco_62xx_write );


DECLARE_LEGACY_DEVICE(NAMCO_62XX, namco_62xx);


#endif	/* NAMCO62_H */
