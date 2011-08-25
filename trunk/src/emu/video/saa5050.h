/*****************************************************************************
 *
 * video/saa5050.h
 *
 * SAA5050
 *
 ****************************************************************************/

#ifndef __SAA5050_H__
#define __SAA5050_H__

#include "devlegcy.h"

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

DECLARE_LEGACY_DEVICE(SAA5050, saa5050);

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_SAA5050_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, SAA5050, 0) \
	MCFG_DEVICE_CONFIG(_interface)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

void saa5050_update(device_t *device, bitmap_t *bitmap, const rectangle *cliprect);
void saa5050_frame_advance(device_t *device);

GFXDECODE_EXTERN( saa5050 );
PALETTE_INIT( saa5050 );

WRITE8_DEVICE_HANDLER( saa5050_videoram_w );
READ8_DEVICE_HANDLER( saa5050_videoram_r );


#endif /* __SAA5050_H__ */
