/*************************************************************************

    Atari Rampart hardware

*************************************************************************/

#include "machine/atarigen.h"

/* shared with arcadecl hardware */
class rampart_state : public atarigen_state
{
public:
	rampart_state(running_machine &machine, const driver_device_config_base &config)
		: atarigen_state(machine, config) { }

	UINT16 *		bitmap;
	UINT8			has_mo;
};


/*----------- defined in video/rampart.c -----------*/

VIDEO_START( rampart );
VIDEO_UPDATE( rampart );

void rampart_bitmap_render(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
