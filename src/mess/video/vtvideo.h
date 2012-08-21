/**********************************************************************

    DEC VT Terminal video emulation
    [ DC012 and DC011 emulation ]

    01/05/2009 Initial implementation [Miodrag Milanovic]

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#ifndef __VT_VIDEO__
#define __VT_VIDEO__

#include "emu.h"
#include "devcb.h"

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

DECLARE_LEGACY_DEVICE(VT100_VIDEO, vt100_video);

#define MCFG_VT100_VIDEO_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, VT100_VIDEO, 0) \
	MCFG_DEVICE_CONFIG(_intrf)
/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _vt_video_interface vt_video_interface;
struct _vt_video_interface
{
	const char *screen_tag;		/* screen we are acting on */
	const char *char_rom_region_tag; /* character rom region */

	/* this gets called for every memory read */
	devcb_read8			in_ram_func;
	devcb_write8		clear_video_interrupt;
};

/***************************************************************************
    PROTOTYPES
***************************************************************************/
/* register access */
READ8_DEVICE_HANDLER  ( vt_video_lba7_r );
WRITE8_DEVICE_HANDLER ( vt_video_dc012_w );
WRITE8_DEVICE_HANDLER ( vt_video_dc011_w );
WRITE8_DEVICE_HANDLER ( vt_video_brightness_w );


/* screen update */
void vt_video_update(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect);
void rainbow_video_update(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect);

#endif
