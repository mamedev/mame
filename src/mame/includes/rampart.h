/*************************************************************************

    Atari Rampart hardware

*************************************************************************/

#include "machine/atarigen.h"

/* shared with arcadecl hardware */
typedef struct _rampart_state rampart_state;
struct _rampart_state
{
	atarigen_state	atarigen;
	UINT16 *		bitmap;
	UINT8			has_mo;
};


/*----------- defined in video/rampart.c -----------*/

VIDEO_START( rampart );
VIDEO_UPDATE( rampart );

void rampart_bitmap_render(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
