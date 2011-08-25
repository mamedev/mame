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

#define MCFG_KANEKO_PANDORA_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, KANEKO_PANDORA, 0) \
	MCFG_DEVICE_CONFIG(_interface)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

void pandora_update(device_t *device, bitmap_t *bitmap, const rectangle *cliprect);
void pandora_eof(device_t *device);
void pandora_set_clear_bitmap(device_t *device, int clear);
void pandora_set_bg_pen( device_t *device, int pen );

WRITE8_DEVICE_HANDLER ( pandora_spriteram_w );
READ8_DEVICE_HANDLER( pandora_spriteram_r );

WRITE16_DEVICE_HANDLER( pandora_spriteram_LSB_w );
READ16_DEVICE_HANDLER( pandora_spriteram_LSB_r );

#endif /* __KAN_PAND_H__ */

