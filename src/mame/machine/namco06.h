#ifndef NAMCO06_H
#define NAMCO06_H

#include "devintrf.h"


typedef struct _namco_06xx_config namco_06xx_config;
struct _namco_06xx_config
{
	const char *nmicpu;
	const char *chip0;
	const char *chip1;
	const char *chip2;
	const char *chip3;
};


#define MDRV_NAMCO_06XX_ADD(_tag, _clock, _nmicpu, _chip0, _chip1, _chip2, _chip3) \
	MDRV_DEVICE_ADD(_tag, NAMCO_06XX, _clock) \
	MDRV_DEVICE_CONFIG_DATAPTR(namco_06xx_config, nmicpu, _nmicpu) \
	MDRV_DEVICE_CONFIG_DATAPTR(namco_06xx_config, chip0, _chip0) \
	MDRV_DEVICE_CONFIG_DATAPTR(namco_06xx_config, chip1, _chip1) \
	MDRV_DEVICE_CONFIG_DATAPTR(namco_06xx_config, chip2, _chip2) \
	MDRV_DEVICE_CONFIG_DATAPTR(namco_06xx_config, chip3, _chip3)


READ8_DEVICE_HANDLER( namco_06xx_data_r );
WRITE8_DEVICE_HANDLER( namco_06xx_data_w );
READ8_DEVICE_HANDLER( namco_06xx_ctrl_r );
WRITE8_DEVICE_HANDLER( namco_06xx_ctrl_w );


/* device get info callback */
#define NAMCO_06XX DEVICE_GET_INFO_NAME(namco_06xx)
DEVICE_GET_INFO( namco_06xx );


#endif
