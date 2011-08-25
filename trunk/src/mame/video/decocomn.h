/*************************************************************************

    decocomn.h

**************************************************************************/

#pragma once
#ifndef __DECOCOMN_H__
#define __DECOCOMN_H__

#include "devcb.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


typedef struct _decocomn_interface decocomn_interface;
struct _decocomn_interface
{
	const char         *screen;
};

DECLARE_LEGACY_DEVICE(DECOCOMN, decocomn);


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_DECOCOMN_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, DECOCOMN, 0) \
	MCFG_DEVICE_CONFIG(_interface)

/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

WRITE16_DEVICE_HANDLER( decocomn_nonbuffered_palette_w );
WRITE16_DEVICE_HANDLER( decocomn_buffered_palette_w );
WRITE16_DEVICE_HANDLER( decocomn_palette_dma_w );

WRITE16_DEVICE_HANDLER( decocomn_priority_w );
READ16_DEVICE_HANDLER( decocomn_priority_r );

READ16_DEVICE_HANDLER( decocomn_71_r );

/* used by boogwing, dassault, nitrobal */
void decocomn_clear_sprite_priority_bitmap(device_t *device);
void decocomn_pdrawgfx(
		device_t *device,
		bitmap_t *dest,const rectangle *clip,const gfx_element *gfx,
		UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,
		int transparent_color,UINT32 pri_mask,UINT32 sprite_mask,UINT8 write_pri,UINT8 alpha);

#endif
