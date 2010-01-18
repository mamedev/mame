/*****************************************************************************
 *
 * video/saa5050.h
 *
 * SAA5050
 *
 ****************************************************************************/

#ifndef __SAA5050_H__
#define __SAA5050_H__


#define SAA5050_VBLANK 2500

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _saa5050_interface saa5050_interface;
struct _saa5050_interface
{
	const char *screen;
	int gfxnum;
	int x, y, size;
	int rev;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

DEVICE_GET_INFO( saa5050 );


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define SAA5050 DEVICE_GET_INFO_NAME( saa5050 )

#define MDRV_SAA5050_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, SAA5050, 0) \
	MDRV_DEVICE_CONFIG(_interface)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

void saa5050_update(running_device *device, bitmap_t *bitmap, const rectangle *cliprect);
void saa5050_frame_advance(running_device *device);

GFXDECODE_EXTERN( saa5050 );
PALETTE_INIT( saa5050 );

WRITE8_DEVICE_HANDLER( saa5050_videoram_w );
READ8_DEVICE_HANDLER( saa5050_videoram_r );


#endif /* __SAA5050_H__ */
