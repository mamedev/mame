/**********************************************************************

    Signetics 2636 video chip

**********************************************************************/

#ifndef __S2636_H__
#define __S2636_H__

#include "devlegcy.h"


#define S2636_IS_PIXEL_DRAWN(p)     (((p) & 0x08) ? TRUE : FALSE)
#define S2636_PIXEL_COLOR(p)        ((p) & 0x07)

/*************************************
 *
 *  Type definitions
 *
 *************************************/

typedef struct _s2636_interface s2636_interface;
struct _s2636_interface
{
	const char *screen;
	int        work_ram_size;
	int        y_offset;
	int        x_offset;
	const char *sound;
};

/*************************************
 *
 *  Device configuration macros
 *
 *************************************/

DECLARE_LEGACY_DEVICE(S2636, s2636);

#define MCFG_S2636_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, S2636, 0) \
	MCFG_DEVICE_CONFIG(_interface)


/*************************************
 *
 *  Device I/O functions
 *
 *************************************/


/* returns a BITMAP_FORMAT_INDEXED16 bitmap the size of the screen
   D0-D2 of each pixel is the pixel color
   D3 indicates whether the S2636 drew this pixel - 0 = not drawn, 1 = drawn */

bitmap_t *s2636_update( device_t *device, const rectangle *cliprect );
WRITE8_DEVICE_HANDLER( s2636_work_ram_w );
READ8_DEVICE_HANDLER( s2636_work_ram_r );


#endif /* __S2636_H__ */
