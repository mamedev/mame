#ifndef NAMCO50_H
#define NAMCO50_H


#define MDRV_NAMCO_50XX_ADD(_tag, _clock) \
	MDRV_DEVICE_ADD(_tag, NAMCO_50XX, _clock) \


READ8_DEVICE_HANDLER( namco_50xx_read );
void namco_50xx_read_request(running_device *device);
WRITE8_DEVICE_HANDLER( namco_50xx_write );


/* device get info callback */
#define NAMCO_50XX DEVICE_GET_INFO_NAME(namco_50xx)
DEVICE_GET_INFO( namco_50xx );


#endif	/* NAMCO50_H */
