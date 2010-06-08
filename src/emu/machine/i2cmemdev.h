/*
 * I2C Memory
 *
 */

#if !defined( I2CMEMDEV_H )
#define I2CMEMDEV_H ( 1 )

#include "devlegcy.h"

#define I2CMEM_E0 ( 1 )
#define I2CMEM_E1 ( 2 )
#define I2CMEM_E2 ( 3 )
#define I2CMEM_SDA ( 5 )
#define I2CMEM_SCL ( 6 )
#define I2CMEM_WC ( 7 )

#define I2CMEM_SLAVE_ADDRESS ( 0xa0 )
#define I2CMEM_SLAVE_ADDRESS_ALT ( 0xb0 )

typedef struct _i2cmem_config i2cmem_config;
struct _i2cmem_config
{
	const int slave_address;
	const int page_size;
	const int data_size;
	const char *data;
};

DECLARE_LEGACY_NVRAM_DEVICE(I2CMEM, i2cmem);

#define MDRV_I2CMEM_ADD(_tag, _slave_address, _page_size, _data_size, _data) \
	MDRV_DEVICE_ADD(_tag, I2CMEM, 0) \
	MDRV_DEVICE_CONFIG_DATA32(i2cmem_config, slave_address, _slave_address) \
	MDRV_DEVICE_CONFIG_DATA32(i2cmem_config, page_size, _page_size) \
	MDRV_DEVICE_CONFIG_DATA32(i2cmem_config, data_size, _data_size) \
	MDRV_DEVICE_CONFIG_DATAPTR(i2cmem_config, data, _data)


void i2cmemdev_write( running_device *device, int line, int data );
int i2cmemdev_read( running_device *device, int line );
void i2cmemdev_set_read_mode( running_device *device, int mode );

#endif
