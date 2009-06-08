/*
 * ATMEL AT28C16
 *
 * 16K ( 2K x 8 ) Parallel EEPROM
 *
 */

#if !defined( AT28C16_H )
#define AT28C16_H ( 1 )

typedef struct _at28c16_config at28c16_config;
struct _at28c16_config
{
	const char *id;
};

#define AT28C16 DEVICE_GET_INFO_NAME(at28c16)
DEVICE_GET_INFO(at28c16);

#define MDRV_AT28C16_ADD(_tag, _id) \
	MDRV_DEVICE_ADD(_tag, AT28C16, 0) \
	MDRV_DEVICE_CONFIG_DATAPTR(at28c16_config, id, _id)


extern void at28c16_a9_12v( const device_config *device, int a9_12v );
extern void at28c16_oe_12v( const device_config *device, int a9_12v );

/* memory handlers */

WRITE8_DEVICE_HANDLER( at28c16_w );
READ8_DEVICE_HANDLER( at28c16_r );

#endif
