#ifndef NAMCO54_H
#define NAMCO54_H

#include "sound/discrete.h"


typedef struct _namco_54xx_config namco_54xx_config;
struct _namco_54xx_config
{
	const char *discrete;	/* name of the discrete sound device */
	int			firstnode;	/* index of the first node */
};


#define MDRV_NAMCO_54XX_ADD(_tag, _clock, _discrete, _firstnode) \
	MDRV_DEVICE_ADD(_tag, NAMCO_54XX, _clock) \
	MDRV_DEVICE_CONFIG_DATAPTR(namco_54xx_config, discrete, _discrete) \
	MDRV_DEVICE_CONFIG_DATA32(namco_54xx_config, firstnode, _firstnode)

#define MDRV_NAMCO_54XX_REMOVE(_tag) \
	MDRV_DEVICE_REMOVE(_tag)


WRITE8_DEVICE_HANDLER( namco_54xx_write );


/* device get info callback */
#define NAMCO_54XX DEVICE_GET_INFO_NAME(namco_54xx)
DEVICE_GET_INFO( namco_54xx );


/* discrete nodes */
#define NAMCO_54XX_0_DATA(base)		(NODE_RELATIVE(base, 0))
#define NAMCO_54XX_1_DATA(base)		(NODE_RELATIVE(base, 1))
#define NAMCO_54XX_2_DATA(base)		(NODE_RELATIVE(base, 2))
#define NAMCO_54XX_P_DATA(base)		(NODE_RELATIVE(base, 3))


#endif	/* NAMCO54_H */
