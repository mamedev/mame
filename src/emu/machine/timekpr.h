/*
 * STmicroelectronics TIMEKEEPER SRAM
 *
 * Supports:
 *           M48T02
 *           M48T35
 *           M48T58
 *           MK48T08
 *
 */

#if !defined( TIMEKPR_H )
#define TIMEKPR_H ( 1 )

#include "devlegcy.h"

typedef struct _timekeeper_config timekeeper_config;
struct _timekeeper_config
{
	const char *data;
};

DECLARE_LEGACY_NVRAM_DEVICE(M48T02, m48t02);

#define MDRV_M48T02_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, M48T02, 0)


DECLARE_LEGACY_NVRAM_DEVICE(M48T35, m48t35);

#define MDRV_M48T35_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, M48T35, 0)


DECLARE_LEGACY_NVRAM_DEVICE(M48T58, m48t58);

#define MDRV_M48T58_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, M48T58, 0)


DECLARE_LEGACY_NVRAM_DEVICE(MK48T08, mk48t08);

#define MDRV_MK48T08_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, MK48T08, 0)


/* memory handlers */

WRITE8_DEVICE_HANDLER( timekeeper_w );
READ8_DEVICE_HANDLER( timekeeper_r );

#endif
