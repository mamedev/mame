#ifndef NAMCO06_H
#define NAMCO06_H

#include "devlegcy.h"


typedef struct _namco_06xx_config namco_06xx_config;
struct _namco_06xx_config
{
	const char *nmicpu;
	const char *chip0;
	const char *chip1;
	const char *chip2;
	const char *chip3;
};


#define MCFG_NAMCO_06XX_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, NAMCO_06XX, _clock) \
	MCFG_DEVICE_CONFIG(_config)


READ8_DEVICE_HANDLER( namco_06xx_data_r );
WRITE8_DEVICE_HANDLER( namco_06xx_data_w );
READ8_DEVICE_HANDLER( namco_06xx_ctrl_r );
WRITE8_DEVICE_HANDLER( namco_06xx_ctrl_w );


/* device get info callback */
DECLARE_LEGACY_DEVICE(NAMCO_06XX, namco_06xx);


#endif
