/*************************************************************************

    Atari Xybots hardware

*************************************************************************/

#include "machine/atarigen.h"

class xybots_state : public atarigen_state
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, xybots_state(machine)); }

	xybots_state(running_machine &machine)
		: atarigen_state(machine) { }

	UINT16			h256;
};


/*----------- defined in video/xybots.c -----------*/

VIDEO_START( xybots );
VIDEO_UPDATE( xybots );
