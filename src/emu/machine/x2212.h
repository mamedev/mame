/*
 * Xicor X2212
 *
 * 256 x 4 bit Nonvolatile Static RAM
 *
 */

#if !defined( X2212_H )
#define X2212_H ( 1 )

typedef struct _x2212_config x2212_config;
struct _x2212_config
{
	const char *data;
};

#define X2212 DEVICE_GET_INFO_NAME(x2212)
DEVICE_GET_INFO(x2212);

#define MDRV_X2212_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, X2212, 0)


extern void x2212_write( running_device *device, int offset, int data );
extern int x2212_read( running_device *device, int offset );
extern void x2212_store( running_device *device, int store );
extern void x2212_array_recall( running_device *device, int array_recall );

#endif
