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

typedef struct _timekeeper_config timekeeper_config;
struct _timekeeper_config
{
	const char *data;
};

#define M48T02 DEVICE_GET_INFO_NAME(m48t02)
DEVICE_GET_INFO(m48t02);

#define MDRV_M48T02_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, M48T02, 0)


#define M48T35 DEVICE_GET_INFO_NAME(m48t35)
DEVICE_GET_INFO(m48t35);

#define MDRV_M48T35_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, M48T35, 0)


#define M48T58 DEVICE_GET_INFO_NAME(m48t58)
DEVICE_GET_INFO(m48t58);

#define MDRV_M48T58_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, M48T58, 0)


#define MK48T08 DEVICE_GET_INFO_NAME(mk48t08)
DEVICE_GET_INFO(mk48t08);

#define MDRV_MK48T08_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, MK48T08, 0)


/* memory handlers */

WRITE8_DEVICE_HANDLER( timekeeper_w );
READ8_DEVICE_HANDLER( timekeeper_r );

#endif
