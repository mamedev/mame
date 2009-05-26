#ifndef NAMCO50_H
#define NAMCO50_H


#define MDRV_NAMCO_50XX_ADD(_tag, _clock) \
	MDRV_DEVICE_ADD(_tag, NAMCO_50XX, _clock) \

#define MDRV_NAMCO_50XX_REMOVE(_tag) \
	MDRV_DEVICE_REMOVE(_tag)


UINT8 namco_50xx_read(const device_config *device);
void namco_50xx_read_request(const device_config *device);
void namco_50xx_write(const device_config *device, UINT8 data);

/* device get info callback */
#define NAMCO_50XX DEVICE_GET_INFO_NAME(namco_50xx)
DEVICE_GET_INFO( namco_50xx );


#endif	/* NAMCO50_H */
