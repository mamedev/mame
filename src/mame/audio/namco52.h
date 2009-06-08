#ifndef NAMCO52_H
#define NAMCO52_H

#include "sound/discrete.h"
#include "devcb.h"


typedef struct _namco_52xx_interface namco_52xx_interface;
struct _namco_52xx_interface
{
	const char *	discrete;	/* name of the discrete sound device */
	int				firstnode;	/* index of the first node */
	attoseconds_t	extclock;	/* external clock period */
	devcb_read8		romread;	/* ROM read handler */
	devcb_read8 	si;			/* SI (pin 6) read handler */
};


#define MDRV_NAMCO_52XX_ADD(_tag, _clock, _interface) \
	MDRV_DEVICE_ADD(_tag, NAMCO_52XX, _clock) \
	MDRV_DEVICE_CONFIG(_interface)


WRITE8_DEVICE_HANDLER( namco_52xx_write );


/* device get info callback */
#define NAMCO_52XX DEVICE_GET_INFO_NAME(namco_52xx)
DEVICE_GET_INFO( namco_52xx );


/* discrete nodes */
#define NAMCO_52XX_P_DATA(base)		(base)


#endif	/* NAMCO52_H */
