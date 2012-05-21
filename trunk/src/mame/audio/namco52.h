#ifndef NAMCO52_H
#define NAMCO52_H

#include "devlegcy.h"
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


#define MCFG_NAMCO_52XX_ADD(_tag, _clock, _interface) \
	MCFG_DEVICE_ADD(_tag, NAMCO_52XX, _clock) \
	MCFG_DEVICE_CONFIG(_interface)


WRITE8_DEVICE_HANDLER( namco_52xx_write );


DECLARE_LEGACY_DEVICE(NAMCO_52XX, namco_52xx);


/* discrete nodes */
#define NAMCO_52XX_P_DATA(base)		(base)


#endif	/* NAMCO52_H */
