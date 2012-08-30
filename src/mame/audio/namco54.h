#ifndef NAMCO54_H
#define NAMCO54_H

#include "devlegcy.h"
#include "sound/discrete.h"


typedef struct _namco_54xx_config namco_54xx_config;
struct _namco_54xx_config
{
	const char *discrete;	/* name of the discrete sound device */
	int			firstnode;	/* index of the first node */
};


#define MCFG_NAMCO_54XX_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, NAMCO_54XX, _clock) \
	MCFG_DEVICE_CONFIG(_config)


WRITE8_DEVICE_HANDLER( namco_54xx_write );


DECLARE_LEGACY_DEVICE(NAMCO_54XX, namco_54xx);


/* discrete nodes */
#define NAMCO_54XX_0_DATA(base)		(NODE_RELATIVE(base, 0))
#define NAMCO_54XX_1_DATA(base)		(NODE_RELATIVE(base, 1))
#define NAMCO_54XX_2_DATA(base)		(NODE_RELATIVE(base, 2))
#define NAMCO_54XX_P_DATA(base)		(NODE_RELATIVE(base, 3))


#endif	/* NAMCO54_H */
