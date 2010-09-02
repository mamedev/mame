/*************************************************************************

    Atari Xybots hardware

*************************************************************************/

#include "machine/atarigen.h"

class xybots_state : public atarigen_state
{
public:
	xybots_state(running_machine &machine, const driver_device_config_base &config)
		: atarigen_state(machine, config) { }

	UINT16			h256;
};


/*----------- defined in video/xybots.c -----------*/

VIDEO_START( xybots );
VIDEO_UPDATE( xybots );
