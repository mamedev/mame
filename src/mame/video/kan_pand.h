/*************************************************************************

    kan_pand.h

    Implementation of Kaneko Pandora sprite chip

**************************************************************************/

#ifndef __KAN_PAND_H__
#define __KAN_PAND_H__

#include "devlegcy.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _kaneko_pandora_interface kaneko_pandora_interface;
struct _kaneko_pandora_interface
{
	const char *screen;
	UINT8      gfx_region;
	int        x;
	int        y;
};

DECLARE_LEGACY_DEVICE(KANEKO_PANDORA, kaneko_pandora);

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_KANEKO_PANDORA_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, KANEKO_PANDORA, 0) \
	MDRV_DEVICE_CONFIG(_interface)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

void pandora_update(running_device *device, bitmap_t *bitmap, const rectangle *cliprect);
void pandora_eof(running_device *device);
void pandora_set_clear_bitmap(running_device *device, int clear);

WRITE8_DEVICE_HANDLER ( pandora_spriteram_w );
READ8_DEVICE_HANDLER( pandora_spriteram_r );

WRITE16_DEVICE_HANDLER( pandora_spriteram_LSB_w );
READ16_DEVICE_HANDLER( pandora_spriteram_LSB_r );

#endif /* __KAN_PAND_H__ */

