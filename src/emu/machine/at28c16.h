/*
 * ATMEL AT28C16
 *
 * 16K ( 2K x 8 ) Parallel EEPROM
 *
 */

#if !defined( AT28C16_H )
#define AT28C16_H ( 1 )

#include "devlegcy.h"

typedef struct _at28c16_config at28c16_config;
struct _at28c16_config
{
	const char *id;
};

DECLARE_LEGACY_NVRAM_DEVICE(AT28C16, at28c16);

#define MDRV_AT28C16_ADD(_tag, _id) \
	MDRV_DEVICE_ADD(_tag, AT28C16, 0) \
	MDRV_DEVICE_CONFIG_DATAPTR(at28c16_config, id, _id)


WRITE_LINE_DEVICE_HANDLER( at28c16_a9_12v );
WRITE_LINE_DEVICE_HANDLER( at28c16_oe_12v );

/* memory handlers */

WRITE8_DEVICE_HANDLER( at28c16_w );
READ8_DEVICE_HANDLER( at28c16_r );

#endif
