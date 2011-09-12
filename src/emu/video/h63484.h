/*************************************************************************

  HD63484 ACRTC
  Advanced CRT Controller.

**************************************************************************/

#ifndef __H63484_H__
#define __H63484_H__

#include "devlegcy.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

DECLARE_LEGACY_MEMORY_DEVICE(H63484, h63484);

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_H63484_ADD(_tag, _clock, _config, _map) \
	MCFG_DEVICE_ADD(_tag, H63484, _clock) \
	MCFG_DEVICE_CONFIG(_config) \
	MCFG_DEVICE_ADDRESS_MAP(AS_0, _map)

#define H63484_INTERFACE(name) \
	const h63484_interface (name) =

typedef void (*h63484_display_pixels_func)(device_t *device, bitmap_t *bitmap, int y, int x, UINT32 address, UINT16 data, UINT8 *vram);
#define H63484_DISPLAY_PIXELS(name) void name(device_t *device, bitmap_t *bitmap, int y, int x, UINT32 address, UINT16 data, UINT8 *vram)


typedef struct _h63484_interface h63484_interface;
struct _h63484_interface
{
	const char *screen_tag;		/* screen we are acting on */
	h63484_display_pixels_func	display_func;
};



/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

READ16_DEVICE_HANDLER( h63484_status_r );
READ16_DEVICE_HANDLER( h63484_data_r );
WRITE16_DEVICE_HANDLER( h63484_address_w );
WRITE16_DEVICE_HANDLER( h63484_data_w );

#endif /* __H63484_H__ */


