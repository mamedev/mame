/*
 * Xicor X2212
 *
 * 256 x 4 bit Nonvolatile Static RAM
 *
 */

#if !defined( X2212_H )
#define X2212_H ( 1 )

/* default nvram contents should be in memory region
 * with the same tag as device.
 */

#define X2212 DEVICE_GET_INFO_NAME(x2212)
DEVICE_GET_INFO(x2212);

#define MDRV_X2212_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, X2212, 0)

WRITE8_DEVICE_HANDLER( x2212_write );
READ8_DEVICE_HANDLER( x2212_read );
WRITE_LINE_DEVICE_HANDLER( x2212_store );
WRITE_LINE_DEVICE_HANDLER( x2212_array_recall );

#endif
