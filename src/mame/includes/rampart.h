/*************************************************************************

    Atari Rampart hardware

*************************************************************************/

#include "machine/atarigen.h"

/* shared with arcadecl hardware */
class rampart_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, rampart_state(machine)); }

	rampart_state(running_machine &machine) { }

	atarigen_state	atarigen;
	UINT16 *		bitmap;
	UINT8			has_mo;
};


/*----------- defined in video/rampart.c -----------*/

VIDEO_START( rampart );
VIDEO_UPDATE( rampart );

void rampart_bitmap_render(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
